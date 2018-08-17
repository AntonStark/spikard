//
// Created by anton on 13.06.18.
//

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"

int main() {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    std::string sum = R"(\sum_{i=0}^\inf a_i)";

    auto data = Parser2::parse(course.getSub(1), sum);
    auto res = data.inputAsCmds;
    std::vector<Parser2::TexCommand> expected = {"\\sum", "_", "{", "i", "=", "0", "}",
                                                 "^", "\\inf", " ", "a", "_", "i"};
    bool passed = (res == expected);
    if (passed)
        std::cout << "PASSED";
    else
        std::cerr << "FAILED";
    return 0;
}
