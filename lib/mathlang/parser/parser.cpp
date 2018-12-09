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

std::vector<LexemeSequence> collectNames(const NameSpaceIndex& index) {
    std::vector<LexemeSequence> storage;
    typedef NameSpaceIndex::NameTy NType;
    for (const auto& t : {NType::MT, NType::SYM, NType::VAR}) {
        const auto& namesThisType = index.getNames(t);
        for (const auto& name : namesThisType)
            storage.emplace_back(name);
    }
    return storage;
}

Parser::Parser(Node* where)
    : _where(where),
      namesDefined(collectNames(where->index())) {}

std::pair<bool, NameGaps> matchWithGaps(const LexemeSequence& input, const LexemeSequence& variant) {
    auto findFirstFrom = [&input] (const Lexeme& find, size_t from) -> size_t {
        for (size_t i = from; i < input.size(); ++i)
            if (input[i] == find)
                return i;
        return size_t(-1);
    };

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
                size_t matchThat = findFirstFrom(nextLexeme, i);
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

std::vector<LexemeSequence> filter(const std::vector<LexemeSequence>& variants, const LexemeSequence& target) {
    std::vector<LexemeSequence> filtered;
    for (const auto& variant : variants) {
        auto result = matchWithGaps(target, variant);
        if (result.first)
            filtered.push_back(variant);
    }
    return filtered;
}

Terms* Parser::parse(CurAnalysisData& source) {
    /// начинаем с одного из слоёв, которые уже не имеют вложенных
    ExpressionLayer* inner = *source.layers.begin();
    LexemeSequence oneLayerLexems = inner->getLexems();
    std::vector<LexemeSequence> filtered = filter(namesDefined, source.filtered);
    for (const auto& f : filtered)
        std::cout << texLexer.print(f) << std::endl;
    /// поддержка областей видимости локальных для символов имён
    std::stack<NameSpaceIndex> symbolsNamespaces;
    size_t debug = filtered.size();
    std::cerr << debug;
}

Terms* parse(Node* where, std::string source) {
    Parser texParser(where);
    auto cad = texLexer.recognize(source);
    return texParser.parse(cad);
}

}
