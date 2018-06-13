//
// Created by anton on 14.06.18.
//

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"

int main() {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    Parser2::Lexer lex(course.getSub(1));

    std::string lim = "\\lim_{n\\rightarrow\\infty} f(x) = \\ell";
    lex.input = lim;
    lex.splitToCmds();
    auto res = lex.inputAsCmds;

    return 0;
}
