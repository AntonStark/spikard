//
// Created by anton on 20.06.18.
//

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"

void print(std::vector<Parser2::TeXCommand> vec, size_t pl) {
    for (const auto& c : vec)
        std::cout << (c._cmd.empty() ? "." : c._cmd) << " ";
    std::cout << "\t\t" << pl << std::endl << std::flush;
}

int main() {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    Parser2::Lexer lex(course.getSub(1));

    std::string brackets = "\\Bigg(\\bigg["
        "x\\mapsto \\delta_1\\sum_{k=1}^s"
        "\\big(v_{l,1}+v_{l,2}+v_{l,3}-a^*_{k,1}-a^*_{k,2}-a^*_{k,3}"
        "\\big)\\bigg],\\\\"
        "\\delta_2\\cdot\\sum_{k=1}^s\\sum_{l=1}^t"
        "det\\Big(a_{k,i}(v_{l,j})\\Big)\\Bigg)";
    lex.input = brackets;
    lex.splitToCmds();
    lex.findBracketPairs();
    auto res = lex.bracketInfo;

    std::vector<std::vector<Parser2::TeXCommand> > subExpressions;
    for (const auto& brackets : res)
        subExpressions.emplace_back(std::next(lex.inputAsCmds.begin(), brackets.first),
                                    std::next(lex.inputAsCmds.begin(), brackets.second+1));

    lex.buildLayerStructure(nullptr, 0, lex.inputAsCmds.size());
    for (const auto& l : lex.layers)
        print(l->_cmds, l->_placeholders);

    return 0;
}
