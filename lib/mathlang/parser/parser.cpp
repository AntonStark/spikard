//
// Created by anton on 05.09.18.
//

#include "parser.hpp"

namespace Parser2
{

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

void collectNames(const NameSpaceIndex& index, std::vector<LexemeSequence>& storage) {
    typedef NameSpaceIndex::NameTy NType;
    for (const auto& t : {NType::MT, NType::SYM, NType::VAR}) {
        const auto& namesThisType = index.getNames(t);
        for (const auto& name : namesThisType)
            storage.emplace_back(name);
    }
}

Parser::Parser(Node* where)
    : _where(where) {
    collectNames(where->index(), namesDefined);
}

/// Отвечает за описание границ имени и его аргументных мест
struct NameGaps
{
    typedef std::pair<size_t, size_t> GapBounds;
    std::vector<GapBounds> _args;
    size_t _ownLength;
    size_t _fullLen;

    explicit NameGaps(size_t ownLength)
        : _ownLength(ownLength), _fullLen(ownLength) {}
    void add(size_t from, size_t to) {
        _args.emplace_back(from, to);
        _fullLen += (to - from);
    }
    size_t getFullLen()
    { return _fullLen; }
};

size_t findFirstFrom(size_t from, const Lexeme& l, const LexemeSequence& input) {
    for (size_t i = from; i < input.size(); ++i)
        if (input[i] == l)
            return i;
    return size_t(-1);
}

/*std::pair<bool, NameGaps> matchWithGaps(const LexemeSequence& input, const LexemeSequence& variant) {
    NameGaps nameGaps(variant.size());
    size_t v = 0;
    for (size_t i = 0; i < input.size() && v < variant.size(); ++i) {
        bool isArgPlace = (texLexer.storage.which(variant[v]._id) == "argument_place");
        if (not isArgPlace) {
            if (variant[v] == input[i])
                ++v;
            else
                return {false, nameGaps};
        } else {
            bool noNextLexeme = (v == variant.size() - 1);
            // пропуск может стоять в конце (напр. \\cdot=\\cdot), тогда сразу успех
            if (noNextLexeme) {
                nameGaps.add(i, input.size());
                break;
            } else {
                const Lexeme& nextLexeme = variant[v+1];
                size_t matchThat = findFirstFrom(i, nextLexeme, input);
                if (matchThat == size_t(-1)) {
                    return {false, nameGaps};
                } else {
                    nameGaps.add(i, matchThat);
                    v+=2;   // впереди совпадение input и variant, проходим "argument_place" и само совпадение
                    i = matchThat;  // ещё будет инкремент в цикле
                }
            }
        }
    }

    return {true, nameGaps};
}*/

std::pair<bool, NameGaps> matchWithGaps(const LexemeSequence& input, const LexemeSequence& variant) {
    NameGaps nameGaps(variant.size());
    size_t i = 0, v = 0;
    while (v < variant.size()) {
        bool inputEnd = (i == input.size());
        if (inputEnd)
            return {false, nameGaps};

        bool isArgPlace = (texLexer.storage.which(variant[v]._id) == "argument_place");
        if (not isArgPlace) {
            if (variant[v] == input[i]) {
                ++i;
                ++v;
            } else
                return {false, nameGaps};
        } else {
            bool noNextLexeme = (v == variant.size() - 1);
            // пропуск может стоять в конце (напр. \\cdot=\\cdot), тогда сразу успех
            if (noNextLexeme) {
                nameGaps.add(i, input.size());
                ++v;
            } else {
                const Lexeme& nextLexeme = variant[v+1];
                size_t matchThat = findFirstFrom(i, nextLexeme, input);
                if (matchThat == size_t(-1)) {
                    return {false, nameGaps};
                } else {
                    nameGaps.add(i, matchThat);
                    v+=2;   // впереди совпадение input и variant, проходим "argument_place" и само совпадение
                    i = matchThat + 1;
                }
            }
        }
    }
    return {true, nameGaps};
}

void filter(const std::vector<LexemeSequence>& variants,
            const LexemeSequence& target,
            std::vector<LexemeSequence> filtered) {
    if (not filtered.empty())
        filtered.clear();
    for (const auto& variant : variants) {
        auto result = matchWithGaps(target, variant);
        if (result.first)
            filtered.push_back(variant);
    }
}

/*
struct IndexLevels {
    typedef std::vector<size_t> IndexLevel;         // набор индексов с одним уровнем индексности
    std::map<int, IndexLevel> _levels;              // отображение level -> набор индексов с тем же level
    std::map<size_t,  size_t> _levelThatPosition;    // отображение индекса LexemeSequence -> level

    IndexLevels(const LexemeSequence& sequence) {
        int level = 1;
        /// строка содержит w, t, b, lc, rc
        for (size_t i = 0; i < sequence.size(); ++i) {
            Token curTok = sequence.at(i)._tok;
            if (curTok == Token::w) {
                auto search = _levels.find(level);
                if (search == _levels.end())        // такого уровня ещё не встречалось
                    _levels[level] = {i};
                else
                    search->second.push_back(i);
            } else if (curTok == Token::t) {
                level *= 2;
                level += 1;
            } else if (curTok == Token::b) {
                level *= 2;
            }

        }
    }
};
 // может помочь в ускорении разбора argument_place
 */
/**
 * Задача данного этапа - построить дерево частичного разбора. Листья такого дерева
 * могут быть как простыми (отдельные имена), так и составные (последовательности имён).
 * Далее останется определить порядок комбинации имён в составных листах и, наконец, собрать терм.
 *
 * Чтобы избежать манипулирования LexemeSequence, имя будет представлено парой
 * чисел [begin, end) - смещения над входной строкой. При сборке терма понадобится обход дерева сверху
 * вниз, значит узел должен иметь список своих потомков.
 *
 * Пока дерево не до конца выстроено будут возникать необработанне куски строки.
 * С точки зрения предыдущего узла они уже его потомки. Но их внутреннее устройство еще не выделено.
 * Таким образом есть четыре вида элементов дерева:
 * 0) "почка" - необработанная строка
 * 1) узел подчинения - имя, список потомков (напр. символ множества)
 * 2) узел сочинения - список потомков (например выражение с операциями)
 * 3) лист - имя
 *
 * храним все элементы в векторе treeStorage и понадобится вектор почек forProcess
 * элемент содержит:
 * 1) границы части строки за которую отвечает
 * 2) LexemeSequence (в случае узла сочинения пустую)
 * 3) список индексов treeStorage (в случае листа пустой)
 *
 * В начале создаём элемент с границами "вся строка" и помещаем в вектор почек
 * разбор идёт пока вектор почек не опустеет
 * при разборе строки, если встречается имя с пропусками, создаём почки для описания
 * пропусков и создаём узел для его описания с потомками - эти почки
 *
 * N.B. создавался один потомок - почка, а преверащается она в набор узлов, соотв. нужно ------ превращается в узел сочинения, всё ок
 *  обновлять список потомков - неправильно, что требуется модификация предка
 * N.B. Что с появлением новых имён внутри выражения? ------ они окажутся в почках для которых не удатся найти ни одного варианта
 * Как однако отличить новое имя от неудачи разбора???
 */

Terms* Parser::parse(CurAnalysisData& source) {
    /// начинаем с одного из слоёв, которые уже не имеют вложенных
    ExpressionLayer* inner = *source.layers.begin();
    LexemeSequence oneLayerLexems = inner->getLexems();
    std::vector<LexemeSequence> filtered;
    filter(namesDefined, source.filtered, filtered);
    /// поддержка областей видимости локальных для символов имён
    std::stack<NameSpaceIndex> symbolsNamespaces;

}

Terms* parse(Node* where, std::string source) {
    Parser texParser(where);
    auto cad = texLexer.recognize(source);
    return texParser.parse(cad);
}

}
