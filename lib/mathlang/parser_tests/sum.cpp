//
// Created by anton on 13.06.18.
//

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"

int main() {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    Parser2::Lexer lex(course.getSub(1));

    std::string sum = "\\sum_{i=0}^\\inf a_i";
    lex.input = sum;
    lex.splitToCmds();
    auto res = lex.inputAsCmds;

    return 0;
}
