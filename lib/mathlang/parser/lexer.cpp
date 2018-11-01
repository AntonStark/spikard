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
            lexems.emplace_back(search->second);
        else {
            auto id = storage.store(cmd);
            std::string category = storage.which(id);
            if (category != "skipping_chars" && category != "bracket_size")
                lexems.emplace_back(storage.store(cmd));
        }

        i = j;
    }
    return ParseStatus();
}

/// Получаем последовательность лексем без skipping_chars и bracket_size // todo вызывать следом за splitTexUnits
void Lexer::filterNotPtintableCmds(const Parser2::LexemeSequence& lexems) {
    // todo перенести сюда код, отбрасывающий skipping_chars и bracket_size
    // todo для сообщений об ошибках нужно знать позицию в исходной строке (до всех фильтраций!) - возможно нужно хранить originOffset в Lexeme
    // а сейчас индекс идёт по lexems где уже нет skipping_chars и bracket_size
}

/// Проверка, есть ли в последовательности команды категории blank
bool Lexer::hasBlanks(const Parser2::LexemeSequence& lexems) {
    return std::any_of(lexems.begin(), lexems.end(),
                       [this] (const Lexeme& l) -> bool
                       { return (storage.which(l._id) == "blank"); });
}

void Lexer::dropBlanks(Parser2::LexemeSequence& lexems) {
    
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
                return {i, "Ошибка: закрывающая скобка " + tokenPrints.at(iTok) +
                           " (" + std::to_string(i) + "-ая TeX-команда) не имеет пары."};
        }
        ++i;
    }
    if (not opened.empty())
        return {opened.top().second, "Ошибка: не найдена закрывающая скобка для " + tokenPrints.at(opened.top().first) +
            " (" + std::to_string(opened.top().second) + "-ая TeX-команда)."};
    return ParseStatus();
}

/// В этой функции строится слоистая структура, описывающая вложенность выражений со скобками
void Lexer::buildLayerStructure(CurAnalysisData* data, ExpressionLayer* target) {
    if (target == nullptr) // стартовый вызов
        target = new ExpressionLayer(data->lexems);

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
        // после _ ^ загрядывать дальше (проверить, что возможно) и если там w, то перескакивать через
        // него, иначе lb обработаем соотв. пунктом
        if (it.tok() == Token::t || it.tok() == Token::b) {    // проверка флага и, возможно, ошибка
            if (ind[it.tok()])                  // такой режим индексности уже был использован
                return ParseStatus(it.pos, std::string("Ошибка: повторное использование ")
                                      + (it.tok() == Token::t ? "верхнего" : "нижнего") + " регистра.");
            ind[it.tok()] = true;

            ++it;
            if (it.end)                         // на смене индексности строка закончилась - ошибка
                return ParseStatus(it.pos - 1, "Ошибка: строка заканчивается командой смены регистра.");

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
    CurAnalysisData buf;
    buf.input = toParse;
    buf.res = splitTexUnits(buf.input, buf.lexems);
    if (!buf.res.success) return buf;

    buf.res = collectBracketInfo(buf.lexems, buf.bracketInfo);
    if (!buf.res.success) return buf;

    Lexer::buildLayerStructure(&buf, nullptr);
    for (auto pL : buf.layers) {
        buf.res = Lexer::checkRegisters(pL);
        if (!buf.res.success) return buf;
    }

    buf.blankFound = hasBlanks(buf.lexems);
    return buf;
}

std::string Lexer::print(const Parser2::LexemeSequence& lSeq) const {
    std::stringstream buf;
    for (const auto& l : lSeq)
        buf << print(l);
    return buf.str();
}

Lexer texLexer = Lexer::configureLatex();

}
