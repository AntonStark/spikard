//
// Created by anton on 01.08.18.
//

#include <string>
#include <algorithm>

#include "../definition.hpp"
#include "../parser/lexer.hpp"

int main() {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    std::string axiom = R"((\forall n\in \mathbb{N})"
                        R"(((n\in M)\Leftrightarrow)"
                        R"((\forall x\in \mathbb{N})"
                        R"((((1+x)** n)\ge (1+(n\times x)))))))";

    PrimaryNode* lecture = course.getSub(1);
    lecture->defType("any");
    lecture->defType("Logical");
    lecture->defSym("\\forall", {"any", "Logical"}, "Logical");
    lecture->defType("\\mathbb{N}");
    lecture->defType("\\mathbb{R}");
    lecture->defType("\\mathbb{N}(");
    lecture->defVar("n", "\\mathbb{N}");
    lecture->defType("Set");
    lecture->defSym("\\in", {"any", "Set"}, "Logical");
    lecture->defVar("M", "Set");
    lecture->defSym("\\Leftrightarrow", {"Logical", "Logical"}, "Logical");
//    lecture->defVar("x", "\\mathbb{N}"); // а x пусть будет нововведённым именем (используется без определения)
    lecture->defConst("1", "\\mathbb{N}");
    lecture->defSym("+", {"\\mathbb{N}", "\\mathbb{N}"}, "\\mathbb{N}");
    lecture->defSym("**", {"\\mathbb{N}", "\\mathbb{N}"}, "\\mathbb{N}");
    lecture->defSym("\\times", {"\\mathbb{N}", "\\mathbb{N}"}, "\\mathbb{N}");
    /*auto data = Parser2::parse(lecture, axiom);
    auto res = data.inputAsCmds;
    std::vector<Parser2::TexCommand> expected = {
        "(", "\\forall", " ", "n", "\\in", " ", "\\mathbb", "{", "N", "}",
        "(", "(", "n", "\\in", " ", "M", ")", "\\Leftrightarrow",
        "(", "\\forall", " ", "x", "\\in", " ", "\\mathbb", "{", "N", "}",
        "(", "(", "(", "1", "+", "x", ")", "*", "*", " ", "n", ")",
            "\\ge", " ", "(", "1", "+", "(", "n", "\\times", " ", "x", ")", ")", ")", ")", ")", ")"};
    Parser2::Lexer::parseNames(&data);
    bool passed = (res == expected);
    if (passed)
        std::cout << "PASSED";
    else
        std::cerr << "FAILED";
    Parser2::LexemeSequence seq;
    auto res = Parser2::splitTexUnits(axiom, seq);*/
    return 0;
}
