//
// Created by anton on 18.08.18.
//

#include <string>

#include "../definition.hpp"
#include "../parser/lexer.hpp"

using namespace Parser2;
int main() {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    std::vector<bool> passed;

    {
        std::string someNewName = R"(1\le i,j\le n)";
        CurAnalysisData data = Parser2::parse(course.getSub(1), someNewName);
        TexSequence res = Lexer::readOneSymbolsCommands(&data, 2);
        TexSequence expected = {"i"};
        passed.push_back(res == expected);
    }

    {
        std::string someNewName = R"(\forall A_i^j\in A)";
        CurAnalysisData data = Parser2::parse(course.getSub(1), someNewName);
        TexSequence res = Lexer::readOneSymbolsCommands(&data, 1);
        TexSequence expected = {"A", "_", "i", "^", "j"};
        passed.push_back(res == expected);
    }

    {
        std::string someNewName = R"(\int_0^1 {x^2})";
        CurAnalysisData data = Parser2::parse(course.getSub(1), someNewName);
        TexSequence res = Lexer::readOneSymbolsCommands(&data, 0);
        TexSequence expected = {"\\int", "_", "0", "^", "1", "{", "x", "^", "2", "}"};
        passed.push_back(res == expected);
    }

    {
        std::string someNewName = R"(\forall A_i _k^j\in A)";
        CurAnalysisData data = Parser2::parse(course.getSub(1), someNewName);
        TexSequence res = Lexer::readOneSymbolsCommands(&data, 1);
        TexSequence expected = {"A", "_", "i"};
        passed.push_back(res == expected);
    }

    for (const auto& p : passed)
        if (p)
            std::cout << "PASSED" << std::endl;
        else
            std::cerr << "FAILED" << std::endl;
    return 0;
}
