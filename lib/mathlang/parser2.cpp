//
// Created by anton on 10.06.18.
//

#include "parser2.hpp"

namespace Parser2 {

std::set<std::string> blankCommands = {"\\<space>", "~", "\\nobreakspace",
    "\\!", "\\,", "\\thinspace", "\\:", "\\medspace",
    "\\;", "\\thickspace", "\\enspace", "\\quad", "\\qquad"};

std::set<std::string> bracketSizeCommands = {"\\left", "\\big", "\\bigl", "\\bigr", "\\middle",
    "\\Big", "\\Bigl", "\\Bigr", "\\right", "\\bigg", "\\biggl", "\\biggr", "\\Bigg", "\\Biggl", "\\Biggr"};

std::map<std::string, Token> structureSymbols = {
    {"^", Token::t}, {"_", Token::b},
    {"(", Token::l}, {")", Token::r},
    {"[", Token::ls},{"]", Token::rs},
    {"{", Token::lc},{"}", Token::rc},
    {",", Token::c}
};

std::map<Token, std::string> tokenPrints = {
    {Token::t, "^"}, {Token::b, "_"},
    {Token::l, "("}, {Token::r, ")"},
    {Token::ls,"["}, {Token::rs,"]"},
    {Token::lc,"{"}, {Token::rc,"}"},
    {Token::c, ","}
};
std::string printToken(const Token& t) {
    if (t == Token::w)
        return "";
    else if (t == Token::s)
        return " ";
    else
        return tokenPrints.at(t);
}

std::set<char> skippingChars = {' ', '\t', '&'};

bool filterTexCommands(const std::string& cmd) {
    if (cmd.length() == 1)
        // опускаем ' ' и другие skippingChars
        return (skippingChars.find(cmd.at(0)) == skippingChars.end());
    else
        // а также размеры скобок
        return (bracketSizeCommands.find(cmd) == bracketSizeCommands.end());
}

/// В этой функции только разбор на команды ТеХ-а и соответствующая проверка корректности
ParseStatus Lexer::splitTexUnits(const std::string& input, LexemeSequence& lexems) {
    auto isAlphaAt = [input] (size_t j) -> bool
    { return std::isalpha(static_cast<unsigned char>(input.at(j))); };

    size_t j, i = 0;
    while (i < input.length()) {
        j = i + 1;
        if (input.at(i) == '\\') {      // тогда далее команда или экранированный символ
            if (j == input.length())    // i-ый символ - конец строки, и строка оканчивается на \ - ошибка
                return {input.length()-1, "Ошибка: пустая TeX-команда."};
            while (j < input.length() && isAlphaAt(j))
                ++j;
            if (j == i + 1)             // значит следом за \ идёт не буква, а экранированный символ
                ++j;
        }

        const std::string& cmd = input.substr(i, j-i);
        auto search = structureSymbols.find(cmd);
        if (search != structureSymbols.end())
            lexems.emplace_back(search->second);
        else if (blankCommands.find(cmd) != blankCommands.end())
            lexems.emplace_back(Token::s);
        else if (filterTexCommands(cmd))
            lexems.emplace_back(i, j-i);

        i = j;
    }

    // todo вызвать функцию, строющую вектор границ символов и проверяющую на двойные индексы

    return ParseStatus();
}
void scanNames(PrimaryNode* node, std::set<std::string>& storage) {
    auto appendSet = [&storage] (const std::set<std::string>& set)
        { storage.insert(set.begin(), set.end()); };

    typedef NameSpaceIndex::NameTy NType;
    const NameSpaceIndex& index = node->index();
    for (const auto& t : {NType::MT, NType::SYM, NType::VAR, NType::CONST})
        appendSet(index.getNames(t));
}
Lexer::Lexer(PrimaryNode* where)
    : _where(where), localNames(where) {
    scanNames(where, namesDefined);
    // todo наполнение definedTexSeq
}

ParseStatus Lexer::collectBracketInfo(const LexemeSequence& lexems, std::map<size_t, size_t>& bracketInfo) {
    auto isOpenBracket  = [] (const Token& t) -> bool
        { return (t == Token::l || t == Token::ls || t == Token::lc); };
    auto isCloseBracket = [] (const Token& t) -> bool
    { return (t == Token::r || t == Token::rs || t == Token::rc); };
    auto isPairBrackets = [] (const Token& open, const Token& some) -> bool {
        return  (open == Token::l  && some == Token::r   ||
                 open == Token::ls && some == Token::rs  ||
                 open == Token::lc && some == Token::rc);
    };
    std::stack<std::pair<Token, size_t> > opened;

    size_t i = 0;
    while (i < lexems.size()) {
        Token iTok = lexems.at(i)._tok;
        if (isOpenBracket(iTok))
            opened.emplace(iTok, i);
        else if (isCloseBracket(iTok)) {
            if (isPairBrackets(opened.top().first, iTok)) {
                bracketInfo.emplace(opened.top().second, i);
                opened.pop();
            }
            else
                return {i, "Ошибка: закрывающая скобка " + printToken(iTok) +
                           " (" + std::to_string(i) + "-ая TeX-команда) не имеет пары."};
        }
        ++i;
    }
    if (not opened.empty())
        return {opened.top().second, "Ошибка: не найдена закрывающая скобка для " + printToken(opened.top().first) +
            " (" + std::to_string(opened.top().second) + "-ая TeX-команда)."};
    return ParseStatus();
}

// Начальный вызов от (nullptr, 0, inputCmdsPrintable.length())
// i - абсолютное смещение по строке, bound индекс правой скобки верхнего слоя
// todo нужно обрабатывать аргументы функций отдельно: разделять по запятым
void Lexer::buildLayerStructure(CurAnalysisData* data, ExpressionLayer* parent, size_t pos, size_t bound) {
    size_t indentInParent = (parent ? parent->_cmds.size() : 0);
    auto cur = new ExpressionLayer(parent, indentInParent);

    auto bracketInfo = data->bracketInfo;
    auto search = bracketInfo.lower_bound(pos);   // находим ближайшую откр. скобку после pos
    while (search != bracketInfo.end() && search->first < bound) {
        size_t j = search->first;
        cur->emplaceBack(data->inputAsCmds, pos, j + 1);

        buildLayerStructure(data, cur, j+1, search->second);
        cur->insertPlaceholder();

        pos = search->second;
        search = bracketInfo.lower_bound(pos);
    }                                           // если такой нет вовсе или она в следующей вложенности - идём до конца
    cur->emplaceBack(data->inputAsCmds, pos, bound);
    data->layers.insert(cur);
}

/*

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
*/

// NB новое имя может возникать и в тех случаях когда множество подходящих имён
// не пусто (напр., множество A определёно, а суммирование ведётся по A_i\in A)
// NB2 ни одно имя не может начинаться со скобки, так что если остановились на скобке - пропускать
// todo имена не должны содержать пробелов: при определении имени ругаться на пробелы.
// todo обрабатывать служебные символы: ( ) , . { } _ ^
/*void Lexer::parseNames(CurAnalysisData* data) {
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
}*/

CurAnalysisData::CurAnalysisData(std::string toParse)
    : input(std::move(toParse)) {
    ParseStatus res;
    res = Lexer::splitTexUnits(input, lexems);
    if (!res.success)
        throw std::invalid_argument(res.mess);
    res = Lexer::collectBracketInfo(lexems, bracketInfo);
    if (!res.success)
        throw std::invalid_argument(res.mess);

//    Lexer::buildLayerStructure(this, nullptr, 0, inputAsCmds.size());
//    Lexer::parseNames(this);
}

CurAnalysisData parse(PrimaryNode* where, std::string toParse) {
    Lexer lexer(where); // fixme задействовать для получения списка определённых имён (как послед. команд)
    return CurAnalysisData(toParse);
}

}
