//
// Created by anton on 14.06.18.
//

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"

int main() {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    std::string lim = "\\lim_{n\\rightarrow\\infty} f(x) = \\ell";

    auto data = Parser2::parse(course.getSub(1), lim);
    auto res = data.inputAsCmds;
    std::vector<Parser2::TexCommand> expected = {"\\lim", "_", "{", "n", "\\rightarrow", "\\infty", "}",
                                                 "f", "(", "x", ")", "=", "\\ell"};
    bool passed = (res == expected);
    if (passed)
        std::cout << "PASSED";
    else
        std::cerr << "FAILED";
    return 0;
}
