//
// Created by anton on 14.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../parser/lexer.hpp"

using namespace std;
using namespace Parser2;

TEST(ParserTests, Limit) {
    string source = R"(\lim_{n\rightarrow\infty} f(x) = \ell)";

    Lexer lex;
    LexemeSequence lexems;
    auto ret = lex.splitTexUnits(source, lexems);

    vector<string> result;
    transform(lexems.begin(), lexems.end(), inserter(result, result.begin()),
              [&lex] (const Lexeme& l) -> string { return (lex.print(l)); });
    
    vector<string> expected = {"\\lim", "_",
                                "{", "n", "\\rightarrow", "\\infty", "}",
                                "f", "(", "x", ")", "=", "\\ell"};
    ASSERT_EQ(result, expected);
}
