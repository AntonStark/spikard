//
// Created by anton on 14.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../parser2.hpp"

using namespace std;
using namespace Parser2;

TEST(ParserTests, Limit) {
    string source = R"(\lim_{n\rightarrow\infty} f(x) = \ell)";

    LexemeSequence lexems;
    auto ret = Lexer::splitTexUnits(source, lexems);

    vector<string> result;
    transform(lexems.begin(), lexems.end(), inserter(result, result.begin()),
              [&source] (const Lexeme& l) -> string {
                  return (l._tok == Token::w ? source.substr(l._pos, l._len) : printToken(l._tok));
              });
    
    vector<string> expected = {"\\lim", "_",
                                "{", "n", "\\rightarrow", "\\infty", "}",
                                "f", "(", "x", ")", "=", "\\ell"};
    ASSERT_EQ(result, expected);
}
