//
// Created by anton on 14.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"


TEST(ParserTests, Limit) {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    std::string lim = R"(\lim_{n\rightarrow\infty} f(x) = \ell)";

    auto data = Parser2::parse(course.getSub(1), lim);
    auto res = data.inputAsCmds;
    std::vector<Parser2::TexCommand> expected = {"\\lim", "_",
                         "{", "n", "\\rightarrow", "\\infty", "}",
                         "f", "(", "x", ")", "=", "\\ell"};
    ASSERT_EQ(res, expected);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
