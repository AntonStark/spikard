//
// Created by anton on 14.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../parser2.hpp"

using namespace std;
using namespace Parser2;

TEST(ParserTests, Integral) {
    string source = R"(\int_a^b \! f(x, y) \, \mathrm{d}x)";

    LexemeSequence lexems;
    auto ret = Lexer::splitTexUnits(source, lexems);

    vector<string> result;
    transform(lexems.begin(), lexems.end(), inserter(result, result.begin()),
              [&source] (const Lexeme& l) -> string {
                    return (l._tok == Token::w ? source.substr(l._pos, l._len) : printToken(l._tok));
    });

    vector<string> expected = {"\\int", "_", "a", "^", "b", " ",
                                "f", "(", "x", ",", "y", ")", " ",
                                "\\mathrm", "{", "d", "}", "x"};
    ASSERT_EQ(result, expected);
}
