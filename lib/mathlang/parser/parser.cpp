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

void scanNames(Node* node, std::set<LexemeSequence>& storage) {
    typedef NameSpaceIndex::NameTy NType;
    const NameSpaceIndex& index = node->index();
    for (const auto& t : {NType::MT, NType::SYM, NType::VAR}) {
        const auto& namesThisType = index.getNames(t);
        storage.insert(namesThisType.begin(), namesThisType.end());
    }
}

Parser::Parser(Node* where)
    : _where(where) {
    scanNames(where, namesDefined);
}

std::set<LexemeSequence> match(std::set<LexemeSequence> variants,
                               const LexemeSequence& oneLayerLexems, const std::vector<int>& lexemIndices) {
    /**
     * выполняем проверку за одих проход.
     * как только нашли, что имя (с пропусками!) не подходит - отбрасываем
     *
     * важный момент: в пропуске не происходит смена уровня индексности
     * это так по определению символа: ведь он - способ связи имён, подстановка их в окошки
     * так что встретив звёздочку, сверяем с тем. что за звёздочкой только лексему с
     * тем же уровнем индексности
     */

}

std::vector<int> calculateIndexLevels(const LexemeSequence& sequence) {
    std::vector<int> indices(sequence.size(), 0);
    // todo собственно уровни (см checkRegisters)

    return indices;
}

Terms* Parser::parse(CurAnalysisData& source) {
    /// начинаем с одного из слоёв, которые уже не имеют вложенных
    ExpressionLayer* inner = *source.layers.begin();
    LexemeSequence oneLayerLexems = inner->getLexems();
    std::vector<int> lexemIndices = calculateIndexLevels(oneLayerLexems);

    /// поддержка областей видимости локальных для символов имён
    std::stack<NameSpaceIndex> symbolsNamespaces;

}

Terms* parse(Node* where, std::string source) {
    Parser texParser(where);
    auto cad = texLexer.recognize(source);
    return texParser.parse(cad);
}

}
