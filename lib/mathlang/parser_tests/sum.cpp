//
// Created by anton on 13.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../rationale.hpp"
#include "../parser2.hpp"


TEST(ParserTests, Sum) {
    BranchNode course("Тестовый");
    course.startLecture("Раз");
    std::string sum = R"(\sum_{i=0}^\inf a_i)";

    auto data = Parser2::parse(course.getSub(1), sum);
    auto res = data.inputAsCmds;
    std::vector<Parser2::TexCommand> expected = {"\\sum", "_", 
                 "{", "i", "=", "0", "}", "^", "\\inf", "a", "_", "i"};
    ASSERT_EQ(res, expected);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
