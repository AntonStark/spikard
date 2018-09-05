//
// Created by anton on 10.06.18.
//

#include "lexer2.hpp"

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
    {"{", Token::lc},{"}", Token::rc}
};

std::map<Token, std::string> tokenPrints = {
    {Token::t, "^"}, {Token::b, "_"},
    {Token::l, "("}, {Token::r, ")"},
    {Token::ls,"["}, {Token::rs,"]"},
    {Token::lc,"{"}, {Token::rc,"}"}
};
std::string printToken(const Token& t) {
    if (t == Token::w)
        return "";
    else
        return tokenPrints.at(t);
}

std::set<char> skippingChars = {' ', '\t', '&'};

void scanNames(PrimaryNode* node, std::set<std::string>& storage) {
    typedef NameSpaceIndex::NameTy NType;
    const NameSpaceIndex& index = node->index();
    for (const auto& t : {NType::MT, NType::SYM, NType::VAR, NType::CONST}) {
        const std::set<std::string>& namesThisType = index.getNames(t);
        storage.insert(namesThisType.begin(), namesThisType.end());
    }
}
Lexer::Lexer(PrimaryNode* where)
    : _where(where), localNames(where) {
    scanNames(where, namesDefined);
    // todo наполнение definedTexSeq
}

bool filterTexCommands(const std::string& cmd) {
    if (cmd.length() == 1)
        // опускаем ' ' и другие skippingChars
        return (skippingChars.find(cmd.at(0)) == skippingChars.end());
    else
        // а также размеры скобок
        return (bracketSizeCommands.find(cmd) == bracketSizeCommands.end());
}

/// В этой функции только разбор на команды ТеХ-а и проверка корректности на \ в конце строки
ParseStatus Lexer::splitTexUnits(const std::string& input, LexemeSequence& lexems) {
    auto isAlphaAt = [input] (size_t j) -> bool
    { return std::isalpha(static_cast<unsigned char>(input.at(j))); };

    size_t j, i = 0;
    while (i < input.length()) {
        j = i + 1;
        if (input.at(i) == '\\') {      // тогда далее команда или экранированный символ
            if (j == input.length())    // i-ый символ - конец строки, и строка оканчивается на \ - ошибка
                return ParseStatus(input.length()-1, "Ошибка: пустая TeX-команда.");
            while (j < input.length() && isAlphaAt(j))
                ++j;
            if (j == i + 1)             // значит следом за \ идёт не буква, а экранированный символ
                ++j;
        }

        const std::string& cmd = input.substr(i, j-i);
        auto search = structureSymbols.find(cmd);
        if (search != structureSymbols.end())
            lexems.emplace_back(input, search->second);
        else if (filterTexCommands(cmd))
            lexems.emplace_back(input, i, j-i);

        i = j;
    }
    return ParseStatus();
}

/// В этой функции строится словарь парных скобок и проверяется правильность их расстановки
ParseStatus Lexer::collectBracketInfo(const LexemeSequence& lexems, std::map<size_t, size_t>& bracketInfo) {
    auto isOpenBracket  = [] (const Token& t) -> bool
        { return (t == Token::l || t == Token::ls || t == Token::lc); };
    auto isCloseBracket = [] (const Token& t) -> bool
    { return (t == Token::r || t == Token::rs || t == Token::rc); };
    auto isPairBrackets = [] (const Token& open, const Token& some) -> bool {
        return ((open == Token::l  && some == Token::r)   ||
                (open == Token::ls && some == Token::rs)  ||
                (open == Token::lc && some == Token::rc));
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

/// В этой функции строится слоистая структура, описывающая вложенность выражений со скобками
void Lexer::buildLayerStructure(CurAnalysisData* data,
    std::pair<size_t, size_t> enclosingBrackets, ExpressionLayer* target) {
    if (enclosingBrackets.second == 0)
        enclosingBrackets.second = data->lexems.size();
    if (target == nullptr) // стартовый вызов
        target = new ExpressionLayer(data->lexems);

    // находим ближайшую откр. скобку после i
    auto nextBracketsAfter = [data] (size_t i) -> std::map<size_t, size_t>::iterator {
        auto innerBracket = data->bracketInfo.upper_bound(i);
        while (innerBracket != data->bracketInfo.end()
                && data->lexems.at(innerBracket->first)._tok == Token::lc)      // игнорируем фигурные скобки
            ++innerBracket;
        return innerBracket;
    };

    // если 0, то стартовый вызов - начиаем с самого начала, иначе пропускаем скобку
    size_t i = (enclosingBrackets.first == 0 ? 0 : enclosingBrackets.first + 1);
    auto innerBracket = nextBracketsAfter(i);
    auto endBI = data->bracketInfo.end();
    while (innerBracket != endBI && innerBracket->first < enclosingBrackets.second) {
        target->emplaceBack(i, innerBracket->first + 1);

        auto innerLayer = target->insertPlaceholder();
        buildLayerStructure(data, *innerBracket, innerLayer);

        i = innerBracket->second; // указатель ставится на закрывающую скобку внутренней пары - её не теряем
        innerBracket = nextBracketsAfter(i);
    }
    // и забираем конец
    target->emplaceBack(i, enclosingBrackets.second);
    data->layers.insert(target);
}



/*
/// Строим карту границ символов в пределах ExpressionLayer и походу проверка на двойные индексы
ParseStatus Lexer::detectSymbolBounds(Parser2::ExpressionLayer* layer) {
    const auto& source = layer->lexems;
    auto& target = layer->symbolBounds;
    // если встречаем переход к индексу, то ставим отметку (или ошибка при повторном) и откладываем
    // частичный распознанный символ в стек, начинаем новый
    struct SymInfo {
        size_t begin;
        std::map<Token, bool> indices;
        SymInfo(size_t i) { reset(i); }

        void reset(size_t i) {
            begin = i;
            indices[Token::t] = indices[Token::b] = false;
        }
    };
    std::stack<SymInfo> buf;

    size_t i = 0;
    SymInfo cur(i);
//    ++i;
    while (i < source.size()) {
        // можем встретить: w, t, b, s, lc, rc
        auto iTok = source.at(i)._tok;
        if (iTok == Token::t || iTok == Token::b) {
            if (cur.indices[iTok])      // такой режим индексности уже был использован
                return ParseStatus(i, std::string("Ошибка: повторное использование ")
                                        + (iTok == Token::t ? "верхнего" : "нижнего") + " регистра.");
            if (i + 1 == source.size()) // на смене индексности строка закончилась - ошибка
                return ParseStatus(i, "Ошибка: строка заканчивается командой смены регистра.");
            cur.indices[iTok] = true;

            // далее либо одна команда, либо открывающая скобка и тогда несколько
            auto nextTok = source.at(i + 1)._tok;
            if (nextTok == Token::w)
                ++i;                    // переходим только через символ смены регистра
            else if (nextTok == Token::lc) {
                buf.push(cur);
                ++i;                    // перепрыгиваем через символ смены индекса и открывающую скобку
                cur.reset(++i);         // и начинаем новый символ
            }
            else    // остаётся возможность rc, но после t или b это ошибка
                return ParseStatus(i + 1, "Ошибка: нет команды после смены регистра.");
        }
        else if (iTok == Token::w) {
            target.emplace(cur.begin, i);
            cur.reset(++i);
        }
        else if (iTok == Token::lc) {   // открывающая скобка
            // если встретили (а не перескочили), значит это случай аргумента команды
            buf.push(cur);
            cur.reset(++i);
        }
        else {  // остаётся закрывающая скобка
            target.emplace(cur.begin, i);
            cur = buf.top();
            buf.pop();
            ++i;
        }
    }
    return ParseStatus();
}
*/


/*TexSequence Lexer::readOneSymbolsCommands(CurAnalysisData* data, size_t from) {
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
}*/

/*bool isPrefix(const TexSequence& sequence, const TexSequence& candidate) {
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
}*/

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
    auto checkResult = [&res] {
        if (!res.success)
            throw std::invalid_argument(res.mess);
    };
    res = Lexer::splitTexUnits(input, lexems);
    checkResult();
    res = Lexer::collectBracketInfo(lexems, bracketInfo);
    checkResult();
    Lexer::buildLayerStructure(this);
    /*for (auto pL : layers) {
        res = Lexer::detectSymbolBounds(pL);
        checkResult();
    }*/
//    Lexer::parseNames(this);
}

CurAnalysisData parse(PrimaryNode* where, std::string toParse) {
    Lexer lexer(where);
    return CurAnalysisData(toParse);
}

}
