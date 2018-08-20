//
// Created by anton on 14.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"


TEST(ParserTests, Integral) {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    std::string integ = R"(\int_a^b \! f(x, y) \, \mathrm{d}x)";

    auto data = Parser2::parse(course.getSub(1), integ);
    auto res = data.inputAsCmds;
    std::vector<Parser2::TexCommand> expected = {"\\int", "_", "a", "^", "b", " ",
                                                 "f", "(", "x", ",", "y", ")", " ",
                                                 "\\mathrm", "{", "d", "}", "x"};
    ASSERT_EQ(res, expected);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
