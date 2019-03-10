//
// Created by anton on 10.06.18.
//

#include "lexer.hpp"

namespace Parser2 {

Lexer Lexer::configureLatex() {
    Lexer lex;
    lex.structureSymbols = {
        {"^", Token::t}, {"_", Token::b},
        {"(", Token::l}, {")", Token::r},
        {"[", Token::ls},{"]", Token::rs},
        {"{", Token::lc},{"}", Token::rc}
    };
    lex.tokenPrints = {
        {Token::t, "^"}, {Token::b, "_"},
        {Token::l, "("}, {Token::r, ")"},
        {Token::ls,"["}, {Token::rs,"]"},
        {Token::lc,"{"}, {Token::rc,"}"}
    };

    lex.storage["blank"] = {"\\<space>", "~", "\\nobreakspace",
                        "\\!", "\\,", "\\thinspace", "\\:", "\\medspace",
                        "\\;", "\\thickspace", "\\enspace", "\\quad", "\\qquad"};
    lex.storage["bracket_size"] = {"\\left", "\\big", "\\bigl", "\\bigr", "\\middle",
                               "\\Big", "\\Bigl", "\\Bigr", "\\right", "\\bigg",
                               "\\biggl", "\\biggr", "\\Bigg", "\\Biggl", "\\Biggr"};
    lex.storage["skipping_chars"] = {" ", "\t", "&"};
    lex.storage["argument_place"] = {"\\cdot"};
    lex.storage["variable_place"] = {"\\_"};
    return lex;
}

bool ExpressionLayer::operator<(const Parser2::ExpressionLayer& two) const
{ return (_excludes.size() != two._excludes.size()
          ? _excludes.size() < two._excludes.size()
          : _bounds < two._bounds); }

ExpressionLayer * ExpressionLayer::insertSublayer(std::pair<size_t, size_t> bounds) {
    size_t spaceN = _excludes.size();
    _excludes.emplace(bounds);
    return new ExpressionLayer(_base, bounds, this, spaceN);
}

LexemeSequence ExpressionLayer::getLexems() {
    LexemeSequence buf;
    size_t at = _bounds.first;
    for (const auto& e : _excludes) {
        for (size_t i = at; i < e.first; ++i)
            buf.emplace_back(_base.at(i));
        at = e.second + 1;
    }
    for (size_t i = at; i <= _bounds.second; ++i)
        buf.emplace_back(_base.at(i));
    return buf;
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
            lexems.emplace_back(search->second, i);
        else
            lexems.emplace_back(storage.store(cmd), i);

        i = j;
    }
    return ParseStatus();
}

/// Получаем последовательность лексем без skipping_chars и bracket_size
void Lexer::filterNotPtintableCmds(const LexemeSequence& lexems, LexemeSequence& filtered) {
    for (const auto& l : lexems) {
        std::string category = storage.which(l._id);
        if (category != "skipping_chars" && category != "bracket_size")
            filtered.push_back(l);
    }
}

/// Проверка, есть ли в последовательности команды категории blank
bool Lexer::hasBlanks(const Parser2::LexemeSequence& lexems) {
    return std::any_of(lexems.begin(), lexems.end(),
                       [this] (const Lexeme& l) -> bool
                       { return (storage.which(l._id) == "blank"); });
}

/// Восстанавливающее преобразование: индексы при blank переносятся на левую команду того же уровня
void Lexer::dropBlanks(Parser2::LexemeSequence& lexems) {
    // todo
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

    std::stack<std::pair<Lexeme, size_t> > opened;
    size_t i = 0;
    while (i < lexems.size()) {
        const Lexeme& l = lexems.at(i);
        Token iTok = l._tok;
        if (isOpenBracket(iTok))
            opened.emplace(l, i);
        else if (isCloseBracket(iTok)) {
            if (isPairBrackets(opened.top().first._tok, iTok)) {
                bracketInfo.emplace(opened.top().second, i);
                opened.pop();
            }
            else
                return {l._originOffset, "Ошибка: закрывающая скобка " + tokenPrints.at(iTok) +
                           " (" + std::to_string(i) + "-ая TeX-команда) не имеет пары."};
        }
        ++i;
    }
    if (not opened.empty()) {
        auto unpairedBracket = opened.top();
        return {unpairedBracket.first._originOffset,
                "Ошибка: не найдена закрывающая скобка для " + tokenPrints.at(unpairedBracket.first._tok) +
                " (" + std::to_string(unpairedBracket.second) + "-ая TeX-команда)."};
    }
    return ParseStatus();
}

/// В этой функции строится слоистая структура, описывающая вложенность выражений со скобками
void Lexer::buildLayerStructure(CurAnalysisData* data, ExpressionLayer* target) {
    if (target == nullptr) // стартовый вызов
        target = new ExpressionLayer(data->filtered);

    // находим ближайшую откр. скобку после i
    auto nextBracketsAfter = [data, &target] (size_t i) -> std::map<size_t, size_t>::iterator {
        auto innerBracket = data->bracketInfo.upper_bound(i);
        while (innerBracket != data->bracketInfo.end() && innerBracket->first < target->_bounds.second) {
            if (data->lexems.at(innerBracket->first)._tok == Token::lc) {   // игнорируем фигурные скобки
                target->bracketPairs.emplace(*innerBracket);
                ++innerBracket;
            }
            else
                return innerBracket;
        }
        return data->bracketInfo.end();
    };

    auto endBI = data->bracketInfo.end();
    size_t i = target->_bounds.first;
    auto innerBracket = nextBracketsAfter(i);
    while (innerBracket != endBI) {
        auto innerLayer = target->insertSublayer(*innerBracket);
        buildLayerStructure(data, innerLayer);

        i = innerBracket->second + 1; // перескочили через вложенную пару скобок
        innerBracket = nextBracketsAfter(i);
    }
    data->layers.insert(target);
}

/// Здесь нужно только проверить, что у команд нет повторных нижних или верхних индексов
ParseStatus checkRegisters(ExpressionLayer* layer, ExpressionLayer::Iter& it, size_t bound) {
    std::map<Token, bool> ind;
    ind[Token::b] = ind[Token::t] = false;
    
    while (!it.end && it.pos <= bound) {
        // после _ ^ заглядывать дальше (проверить, что возможно) и если там w, то
        // перескакивать через него, иначе lb обработаем соотв. пунктом
        if (it.tok() == Token::t || it.tok() == Token::b) {    // проверка флага и, возможно, ошибка
            if (ind[it.tok()])                  // такой режим индексности уже был использован
                return ParseStatus(it.get()._originOffset, std::string("Ошибка: повторное использование ")
                                      + (it.tok() == Token::t ? "верхнего" : "нижнего") + " регистра.");
            ind[it.tok()] = true;

            ++it;
            if (it.end)                         // на смене индексности строка закончилась - ошибка
                return ParseStatus(it.get()._originOffset, "Ошибка: строка заканчивается командой смены регистра.");

            if (it.tok() == Token::w)
                ++it;
        } else if (it.tok() == Token::lc) {     // рекурсивная проверка диапазона и переход на-после rc
            // todo нужно различать {} в регистре и строчные. Вторые сами выступают как w - обнуление флагов
            size_t pair = layer->bracketPairs[it.pos];
            auto res = checkRegisters(layer, ++it, pair);
            if (!res.success)
                return res;
        } else {                                // все остальные токены обрабатываются также как w
            ind[Token::b] = ind[Token::t] = false;
            ++it;
        }
    }
    return ParseStatus();
}

ParseStatus Lexer::checkRegisters(Parser2::ExpressionLayer* layer) {
    auto it = layer->begin();
    return Parser2::checkRegisters(layer, it, layer->_bounds.second);
}

CurAnalysisData Lexer::recognize(const std::string& toParse) {
    CurAnalysisData cad;
    cad.input = toParse;
    cad.res = splitTexUnits(cad.input, cad.lexems);
    if (!cad.res.success) return cad;
    filterNotPtintableCmds(cad.lexems, cad.filtered);

    cad.res = collectBracketInfo(cad.filtered, cad.bracketInfo);
    if (!cad.res.success) return cad;

    Lexer::buildLayerStructure(&cad, nullptr);
    for (auto pL : cad.layers) {
        cad.res = Lexer::checkRegisters(pL);
        if (!cad.res.success) return cad;
    }

    cad.blankFound = hasBlanks(cad.lexems); // todo после переноса чистящих трансформаций наверх этот код уже не потребуется здесь
    return cad;
}

std::string Lexer::print(const Parser2::LexemeSequence& lSeq) const {
    std::stringstream buf;
    for (const auto& l : lSeq)
        buf << print(l);
    return buf.str();
}

std::string Lexer::print(const Parser2::LexemeSequence& lSeq, std::pair<size_t, size_t> bounds) const {
    std::stringstream buf;
    for (size_t i = bounds.first; i < bounds.second; ++i)
        buf << print(lSeq[i]);
    return buf.str();
}

}
