//
// Created by anton on 10.06.18.
//

#include "parser2.hpp"

namespace Parser2 {

std::set<TeXCommand> texBrackets =
    {"{", "}", "(", ")", "[", "]"};
std::map<TeXCommand, TeXCommand>  pairBrackets =
    { {"{", "}"}, {"(", ")"}, {"[", "]"} };

bool isOpenBracket(TeXCommand cmd)
{ return (pairBrackets.find(cmd) != pairBrackets.end()); }


void Lexer::splitToCmds(CurAnalysisData* data) {
    auto input = data->input;
    size_t j, i = 0;
    while (i < input.length()) {
        j = i + 1;
        if (input.at(i) == '\\') {  // команда или экранированный символ
            while (j < input.length() && std::isalpha(static_cast<unsigned char>(input.at(j))))
                ++j;
            if (j == i + 1)         // следом за \ идёт не буква, а экранированный символ
                ++j;
        }

        data->inputAsCmds.emplace_back(input.substr(i, j-i));
        i = j;
    }
}

size_t Lexer::findFirstBracketFrom(std::vector<TeXCommand> inputAsCmds, size_t pos) {
    while (pos < inputAsCmds.size())
        if (texBrackets.find(inputAsCmds[pos]) != texBrackets.end())
            return pos;
        else
            ++pos;
    return size_t(-1);
}

std::pair<size_t, std::string> Lexer::findBracketPairs(CurAnalysisData* data) {
    std::stack<std::pair<TeXCommand, size_t> > opened;
    size_t i = 0;
    while ((i = findFirstBracketFrom(data->inputAsCmds, i)) != size_t(-1)) {
        TeXCommand iCmd = data->inputAsCmds[i];
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

// Начальный вызов от (nullptr, 0, inputAsCmds.length())
// i - абсолютное смещение по строке, bound индекс правой скобки верхнего слоя
void CurAnalysisData::copyCmds(std::vector<TeXCommand>& target, size_t begin, size_t end) {
    for (size_t i = begin; i < end; ++i)
        target.emplace_back(inputAsCmds.at(i));
}
void Lexer::buildLayerStructure(CurAnalysisData* data, ExpressionLayer* parent, size_t i, size_t bound) {
    size_t indentInParent = (parent ? parent->_cmds.size() : 0);
    auto cur = new ExpressionLayer(parent, indentInParent);
    data->layers.insert(cur);

    auto bracketInfo = data->bracketInfo;
    auto search = bracketInfo.lower_bound(i);                   // находим ближайшую откр. скобку после данной позиции
    while (search != bracketInfo.end() && search->first < bound) {
        size_t j = search->first;
        data->copyCmds(cur->_cmds, i, j);

        buildLayerStructure(data, cur, j+1, search->second);
        cur->insertPlaceholder();

        i = search->second + 1;
        search = bracketInfo.lower_bound(i);
    }                                           // если такой нет вовсе или она в следующей вложенности - идём до конца
    data->copyCmds(cur->_cmds, i, bound);
}

CurAnalysisData parse(PrimaryNode* where, std::string toParse)
{ return CurAnalysisData(where, toParse); }

}
