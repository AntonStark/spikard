//
// Created by anton on 10.06.18.
//

#include "parser2.hpp"

namespace Parser2 {

std::set<TexCommand> texBrackets =
    {"{", "}", "(", ")", "[", "]"};
std::map<TexCommand, TexCommand>  pairBrackets =
    { {"{", "}"}, {"(", ")"}, {"[", "]"} };

bool isOpenBracket(TexCommand cmd)
{ return (pairBrackets.find(cmd) != pairBrackets.end()); }

std::set<TexCommand> unprintable = {" ", "\\<space>", "~", "\\nobreakspace",
    "\\!", "\\,", "\\thinspace", "\\:", "\\medspace",
    "\\;", "\\thickspace", "\\enspace", "\\quad", "\\qquad", "&",
    "\\left", "\\big", "\\bigl", "\\bigr", "\\middle", "\\Big", "\\Bigl", "\\Bigr",
    "\\right", "\\bigg", "\\biggl", "\\biggr", "\\Bigg", "\\Biggl", "\\Biggr"};


TexSequence Lexer::splitToCmds(CurAnalysisData* data) {
    auto input = data->input;
    std::vector<TexCommand> buffer;
    size_t j, i = 0;
    while (i < input.length()) {
        j = i + 1;
        if (input.at(i) == '\\') {  // команда или экранированный символ
            while (j < input.length() && std::isalpha(static_cast<unsigned char>(input.at(j))))
                ++j;
            if (j == i + 1)         // следом за \ идёт не буква, а экранированный символ
                ++j;
        }

        buffer.emplace_back(input.substr(i, j-i));
        i = j;
    }
    buffer = Lexer::eliminateUnprintable(buffer);
    return buffer;
}

std::pair<size_t, std::string> Lexer::checkForTexErrors(CurAnalysisData* data) {
    auto inputAsCmds = data->inputAsCmds;
    // хвостовой \, двойное __ и ^^
    if (inputAsCmds.back()._cmd == "\\")
        return {inputAsCmds.size()-1, "Ошибка: пустая TeX-команда."};
    for (size_t i = 1; i < inputAsCmds.size(); ++i) {
        if (inputAsCmds.at(i)._cmd == "_" && inputAsCmds.at(i-1)._cmd == "_")
            return {i, "Ошибка: повторный _."};
        else if (inputAsCmds.at(i)._cmd == "^" && inputAsCmds.at(i-1)._cmd == "^")
            return {i, "Ошибка: повторный ^."};
    }
    return {size_t(-1), ""};
}

TexSequence Lexer::eliminateUnprintable(TexSequence& inputAsCmds) {
    TexSequence buffer;
    for (const auto& c : inputAsCmds)
        if (unprintable.find(c) == unprintable.end())
            buffer.push_back(c);
    return buffer;
}

size_t Lexer::findFirstBracketFrom(const TexSequence& inputAsCmds, size_t pos) {
    while (pos < inputAsCmds.size())
        if (texBrackets.find(inputAsCmds[pos]) != texBrackets.end())
            return pos;
        else
            ++pos;
    return size_t(-1);
}

std::pair<size_t, std::string> Lexer::findBracketPairs(CurAnalysisData* data) {
    std::stack<std::pair<TexCommand, size_t> > opened;
    const TexSequence& inputAsCmds = data->inputAsCmds;
    size_t i = 0;
    while ((i = findFirstBracketFrom(inputAsCmds, i)) != size_t(-1)) {
        TexCommand iCmd = inputAsCmds[i];
        if (isOpenBracket(iCmd))
            opened.emplace(iCmd, i);
        else if (iCmd == pairBrackets.at(opened.top().first)) {
            data->bracketInfo.emplace(opened.top().second, i);
            opened.pop();
        }
        else
            return {i, "Ошибка: закрывающая скобка " + iCmd._cmd +
                " (" + std::to_string(i) + "-ая TeX-команда) не имеет пары."};
        ++i;
    }
    if (not opened.empty())
        return {opened.top().second, "Ошибка: не найдена закрывающая скобка для " + opened.top().first._cmd +
            " (" + std::to_string(opened.top().second) + "-ая TeX-команда)."};
    return {size_t(-1), ""};
}

void Lexer::parseNames(CurAnalysisData* data) {
    
}

// Начальный вызов от (nullptr, 0, inputCmdsPrintable.length())
// i - абсолютное смещение по строке, bound индекс правой скобки верхнего слоя
void Lexer::buildLayerStructure(CurAnalysisData* data, ExpressionLayer* parent, size_t i, size_t bound) {
    size_t indentInParent = (parent ? parent->_cmds.size() : 0);
    auto cur = new ExpressionLayer(parent, indentInParent);

    auto bracketInfo = data->bracketInfo;
    auto search = bracketInfo.lower_bound(i);                   // находим ближайшую откр. скобку после данной позиции
    while (search != bracketInfo.end() && search->first < bound) {
        size_t j = search->first;
        data->copyCmds(cur->_cmds, i, j+1);

        buildLayerStructure(data, cur, j+1, search->second);
        cur->insertPlaceholder();

        i = search->second;
        search = bracketInfo.lower_bound(i);
    }                                           // если такой нет вовсе или она в следующей вложенности - идём до конца
    data->copyCmds(cur->_cmds, i, bound);
    data->layers.insert(cur);
}

inline void insertSet(std::set<std::string>& target, const std::set<std::string>& set) {
    for (const auto& v : set)
        target.insert(v);
}
std::set<std::string> getAllNames(const NameSpaceIndex& index) {
    std::set<std::string> buf;
    insertSet(buf, index.getNames(NameSpaceIndex::NameTy::MT));
    insertSet(buf, index.getNames(NameSpaceIndex::NameTy::SYM));
    insertSet(buf, index.getNames(NameSpaceIndex::NameTy::VAR));
    insertSet(buf, index.getNames(NameSpaceIndex::NameTy::CONST));
    return buf;
}
CurAnalysisData::CurAnalysisData(PrimaryNode* where, std::string toParse)
    : _where(where), localNames(where), namesDefined(getAllNames(localNames.index())),
      input(std::move(toParse)) {
    inputAsCmds = Lexer::splitToCmds(this);
    Lexer::checkForTexErrors(this);
    Lexer::parseNames(this);
    Lexer::findBracketPairs(this); // todo нужно обрабатывать аргументы функций отдельно: разделять по запятым
    Lexer::buildLayerStructure(this, nullptr, 0, inputAsCmds.size());
}

CurAnalysisData parse(PrimaryNode* where, std::string toParse)
{ return CurAnalysisData(where, toParse); }

}
