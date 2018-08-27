//
// Created by anton on 10.06.18.
//

#include "parser2.hpp"

namespace Parser2 {

std::set<TexCommand> texBrackets =
    {"{", "}", "(", ")", "[", "]"};
std::map<TexCommand, TexCommand>  pairBrackets =
    { {"{", "}"}, {"(", ")"}, {"[", "]"} };

// fixme "&" вовсе не отступ, но его тоже можно заменять пробелом при работе
std::set<TexCommand> blankCommands = {" ", "\\<space>", "~", "\\nobreakspace",
    "\\!", "\\,", "\\thinspace", "\\:", "\\medspace",
    "\\;", "\\thickspace", "\\enspace", "\\quad", "\\qquad", "&"};

std::set<TexCommand> bracketSizeCommands = {"\\left", "\\big", "\\bigl", "\\bigr", "\\middle",
    "\\Big", "\\Bigl", "\\Bigr", "\\right", "\\bigg", "\\biggl", "\\biggr", "\\Bigg", "\\Biggl", "\\Biggr"};


TexSequence Lexer::splitToCmds(const std::string& input) {
    TexSequence buffer;
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
    buffer = Lexer::eliminateSpaces(buffer);
    buffer = Lexer::normalizeBlank(buffer);
    buffer = Lexer::eliminateBracketSizeCommands(buffer);
    return buffer;
}

std::pair<size_t, std::string> Lexer::checkForTexErrors(const TexSequence& source) {
    /// Проверяемые ошибки: хвостовой \, двойное __ и ^^
    if (source.back()._cmd == "\\")
        return {source.size()-1, "Ошибка: пустая TeX-команда."};
    for (size_t i = 1; i < source.size(); ++i) {
        if (source.at(i)._cmd == "_" && source.at(i-1)._cmd == "_")
            return {i, "Ошибка: повторный _."};
        else if (source.at(i)._cmd == "^" && source.at(i-1)._cmd == "^")
            return {i, "Ошибка: повторный ^."};
    }
    return {size_t(-1), ""};
}

// todo выделяется несколько функций фильтрации inputAsCmds, потенциал для рефакторинга
TexSequence Lexer::eliminateSpaces(const TexSequence& texSequence) {
    TexSequence buffer;
    for (const auto& c : texSequence)
        if (c != " ")
            buffer.push_back(c);
    return buffer;
}

TexSequence Lexer::eliminateBracketSizeCommands(const TexSequence& texSequence) {
    auto isBracketSizeCmd = [] (const TexCommand& c) -> bool
    { return (bracketSizeCommands.find(c) != bracketSizeCommands.end()); };
    TexSequence buffer;
    for (const auto& c : texSequence)
        if (!isBracketSizeCmd(c))
            buffer.push_back(c);
    return buffer;
}

TexSequence Lexer::normalizeBlank(const TexSequence& texSequence) {
    TexCommand space(" ");
    auto isBlank = [] (const TexCommand& c) -> bool
                        { return (blankCommands.find(c) != blankCommands.end()); };
    TexSequence buffer;
    for (const auto& c : texSequence)
        if (!isBlank(c))
            buffer.push_back(c);
        else if (buffer.back() != space)
            buffer.push_back(space);
    return buffer;
}

size_t findFirstBracketFrom(const TexSequence& inputAsCmds, size_t pos) {
    while (pos < inputAsCmds.size())
        if (texBrackets.find(inputAsCmds[pos]) != texBrackets.end())
            return pos;
        else
            ++pos;
    return size_t(-1);
}

// todo нужно обрабатывать аргументы функций отдельно: разделять по запятым
std::pair<size_t, std::string> Lexer::collectBracketInfo(CurAnalysisData* data) {
    auto isOpenBracket = [] (const TexCommand& c) -> bool
    { return (pairBrackets.find(c) != pairBrackets.end()); };
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

TexSequence Lexer::readOneSymbolsCommands(CurAnalysisData* data, size_t from) {
    TexSequence source = data->inputAsCmds;
    auto bracketInfo = data->bracketInfo;
    size_t i = from;
    if (source.empty() || i > source.size())
        return {};
    ++i; // одну команду берём в любом случае

    TexCommand group("{");
    std::set<TexCommand> indMod = {"_", "^"};       // идея в том, что если одну форму индекса
                                                    // встретили, больше её быть не может
    if (i >= source.size()) return TexSequence(std::next(source.begin(), from), source.end());
    auto searchIndMod = indMod.find(source.at(i));
    while (searchIndMod != indMod.end()) {
        ++i;        // съели индекс
        if (i >= source.size()) return TexSequence(std::next(source.begin(), from), source.end());
        if (source.at(i) != group)
            ++i;    // съели одиночный индексный аргумент
        else
            i = bracketInfo[i]+1;   // переходим на следующую после скобки команду
        indMod.erase(searchIndMod);
        if (i >= source.size()) return TexSequence(std::next(source.begin(), from), source.end());
        searchIndMod = indMod.find(source.at(i));
    }
    while (i < source.size() && source.at(i) == group)
        i = bracketInfo[i]+1;

    return TexSequence(std::next(source.begin(), from),
                       (i >= source.size() ? source.end() : std::next(source.begin(), i)));
}

bool isPrefix(const TexSequence& sequence, const TexSequence& candidate) {
    if (candidate.size() > sequence.size())
        return false;
    auto ce = candidate.end();
    for (auto cit = candidate.begin(), sit = sequence.begin(); cit != ce; ++cit, ++sit)
        if (*cit != *sit)
            return false;
    return true;
}

std::set<TexSequence> Lexer::selectSuitableWithIndent(const std::set<TexSequence>& definedTexSeq, size_t indent,
                                                      const TexSequence& source) {
    TexSequence indented(next(source.begin(), indent), source.end());
    auto lb = lower_bound(definedTexSeq.begin(), definedTexSeq.end(), indented);
    auto from = prev(lb);
    while (isPrefix(indented, *from))   // найти где перестанет (при обходе к началу) выполняться префиксность
        --from;                         // имён относительно indent (возможно сразу же)
    from = next(from);             // и взять следующий, тогда [from, lb) будет содержать префиксы
    return std::set<TexSequence>(from, lb);
}

// NB новое имя может возникать и в тех случаях когда множество подходящих имён
// не пусто (напр., множество A определёно, а суммирование ведётся по A_i\in A)
// NB2 ни одно имя не может начинаться со скобки, так что если остановились на скобке - пропускать
// todo имена не должны содержать пробелов: при определении имени ругаться на пробелы.
// todo обрабатывать служебные символы: ( ) , . { } _ ^
void Lexer::parseNames(CurAnalysisData* data) {
    TexSequence source = data->inputAsCmds;
    std::priority_queue<PartialResolved> hypotheses;
    hypotheses.emplace(0, PartialResolved::result_type());
    std::set<PartialResolved::result_type> results;

    while (!hypotheses.empty()) {
        PartialResolved res = hypotheses.top();
        hypotheses.pop();
        if (res.indent == source.size()) { // условие успешного завершения ветвления
            results.insert(res.recognized);
            continue;
        }

        // далее ветвление по подходящим именам и символу
        auto suitableWords = selectSuitableWithIndent(data->definedTexSeq, res.indent, source);
        suitableWords.insert(readOneSymbolsCommands(data, res.indent));
        for (const auto& w : suitableWords) {
            size_t newIndent = res.indent + w.size();
            PartialResolved::result_type branch = res.recognized;
            branch.push_back(w);
            hypotheses.emplace(newIndent, branch);
        }
    }
    auto debug = 1;
}

// Начальный вызов от (nullptr, 0, inputCmdsPrintable.length())
// i - абсолютное смещение по строке, bound индекс правой скобки верхнего слоя
void Lexer::buildLayerStructure(CurAnalysisData* data, ExpressionLayer* parent, size_t i, size_t bound) {
    size_t indentInParent = (parent ? parent->_cmds.size() : 0);
    auto cur = new ExpressionLayer(parent, indentInParent);

    auto bracketInfo = data->bracketInfo;
    auto search = bracketInfo.lower_bound(i);   // находим ближайшую откр. скобку после данной позиции
    while (search != bracketInfo.end() && search->first < bound) {
        size_t j = search->first;
        cur->emplaceBack(data->inputAsCmds, i, j + 1);

        buildLayerStructure(data, cur, j+1, search->second);
        cur->insertPlaceholder();

        i = search->second;
        search = bracketInfo.lower_bound(i);
    }                                           // если такой нет вовсе или она в следующей вложенности - идём до конца
    cur->emplaceBack(data->inputAsCmds, i, bound);
    data->layers.insert(cur);
}

std::set<std::string> getAllNames(const NameSpaceIndex& index) {
    std::set<std::string> buf;
    auto appendBuf = [&buf] (const std::set<std::string>& set)
        { buf.insert(set.begin(), set.end()); };
    appendBuf(index.getNames(NameSpaceIndex::NameTy::MT));
    appendBuf(index.getNames(NameSpaceIndex::NameTy::SYM));
    appendBuf(index.getNames(NameSpaceIndex::NameTy::VAR));
    appendBuf(index.getNames(NameSpaceIndex::NameTy::CONST));
    return buf;
}
CurAnalysisData::CurAnalysisData(PrimaryNode* where, std::string toParse)
    : _where(where), localNames(where), namesDefined(getAllNames(localNames.index())),
      input(std::move(toParse)) {
    std::transform(namesDefined.begin(), namesDefined.end(),
        std::inserter(definedTexSeq, definedTexSeq.begin()), Lexer::splitToCmds);
    inputAsCmds = Lexer::splitToCmds(input);

    Lexer::checkForTexErrors(inputAsCmds);
    Lexer::collectBracketInfo(this);

    Lexer::parseNames(this);
    Lexer::buildLayerStructure(this, nullptr, 0, inputAsCmds.size());
}

CurAnalysisData parse(PrimaryNode* where, std::string toParse)
{ return CurAnalysisData(where, toParse); }

}
