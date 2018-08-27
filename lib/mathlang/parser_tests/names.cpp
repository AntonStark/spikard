//
// Created by anton on 01.08.18.
//

#include <string>
#include <algorithm>

#include "../rationale.hpp"
#include "../parser2.hpp"

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
    auto data = Parser2::parse(lecture, axiom);
    auto res = data.inputAsCmds;
    std::vector<Parser2::TexCommand> expected = {
        "(", "\\forall", " ", "n", "\\in", " ", "\\mathbb", "{", "N", "}",
        "(", "(", "n", "\\in", " ", "M", ")", "\\Leftrightarrow",
        "(", "\\forall", " ", "x", "\\in", " ", "\\mathbb", "{", "N", "}",
        "(", "(", "(", "1", "+", "x", ")", "*", "*", " ", "n", ")",
            "\\ge", " ", "(", "1", "+", "(", "n", "\\times", " ", "x", ")", ")", ")", ")", ")", ")"};
    /*auto ub = std::upper_bound(data.definedTexSeq.begin(), data.definedTexSeq.end(), Parser2::TexSequence({"\\mathbb"}));
    auto er = std::equal_range(data.definedTexSeq.begin(), data.definedTexSeq.end(), Parser2::TexSequence({"\\mathbb"}),
        [] (const Parser2::TexSequence& one, const Parser2::TexSequence& two) -> bool {
        if (one.size() <= two.size()) {
            auto twoBeg = Parser2::TexSequence(two.begin(), std::next(two.begin(), one.size()));
            return (one == twoBeg);
        }
        else {
            auto oneBeg = Parser2::TexSequence(one.begin(), std::next(one.begin(), two.size()));
            return (oneBeg == two);
        }
    }); // fixme эта идея не срабатывает, привет перебор :(*/
    Parser2::Lexer::parseNames(&data);
    bool passed = (res == expected);
    if (passed)
        std::cout << "PASSED";
    else
        std::cerr << "FAILED";
    return 0;
}
