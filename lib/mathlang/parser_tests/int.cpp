//
// Created by anton on 14.06.18.
//

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"

int main() {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    std::string integ = "\\int_a^b \\! f(x) \\, \\mathrm{d}x";

    auto data = Parser2::parse(course.getSub(1), integ);
    auto res = data.inputAsCmds;
    std::vector<Parser2::TeXCommand> expected = {"\\int", "_", "a", "^", "b", " ", "\\!", " ",
                                                 "f", "(", "x", ")", " ", "\\,", " ",
                                                 "\\mathrm", "{", "d", "}", "x"};
    bool passed = (res == expected);
    if (passed)
        std::cout << "PASSED";
    else
        std::cerr << "FAILED";
    return 0;
}
