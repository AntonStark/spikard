//
// Created by anton on 13.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../parser/lexer.hpp"

using namespace std;
using namespace Parser2;

TEST(ParserTests, Sum) {
    std::string source = R"(\sum_{i=0}^\inf a_i)";

    Lexer lex;
    LexemeSequence lexems;
    auto ret = lex.splitTexUnits(source, lexems);

    vector<string> result;
    transform(lexems.begin(), lexems.end(), inserter(result, result.begin()),
              [&lex] (const Lexeme& l) -> string { return (lex.print(l)); });
    
    vector<string> expected = {"\\sum", 
                       "_", "{", "i", "=", "0", "}", 
                       "^", "\\inf", "a", "_", "i"};
    ASSERT_EQ(result, expected);
}
