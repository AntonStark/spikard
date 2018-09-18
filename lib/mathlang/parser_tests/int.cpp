//
// Created by anton on 14.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../lexer2.hpp"

using namespace std;
using namespace Parser2;

TEST(ParserTests, Integral) {
    string source = R"(\int_a^b \! f(x, y) \, \mathrm{d}x)";

    Lexer lex;
    LexemeSequence lexems;
    auto ret = lex.splitTexUnits(source, lexems);

    vector<string> result;
    transform(lexems.begin(), lexems.end(), inserter(result, result.begin()),
              [&lex] (const Lexeme& l) -> string { return (lex.print(l)); });

    vector<string> expected = {"\\int", "_", "a", "^", "b", "\\!",
                                "f", "(", "x", ",", "y", ")", "\\,",
                                "\\mathrm", "{", "d", "}", "x"};
    ASSERT_EQ(result, expected);
}
