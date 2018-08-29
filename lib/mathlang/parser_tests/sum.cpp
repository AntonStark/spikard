//
// Created by anton on 13.06.18.
//

#include "gtest/gtest.h"

#include <string>

#include "../parser2.hpp"

using namespace std;
using namespace Parser2;

TEST(ParserTests, Sum) {
    std::string source = R"(\sum_{i=0}^\inf a_i)";

    LexemeSequence lexems;
    auto ret = Lexer::splitTexUnits(source, lexems);

    vector<string> result;
    transform(lexems.begin(), lexems.end(), inserter(result, result.begin()),
              [&source] (const Lexeme& l) -> string {
                  return (l._tok == Token::w ? source.substr(l._pos, l._len) : printToken(l._tok));
              });
    
    vector<string> expected = {"\\sum", 
                       "_", "{", "i", "=", "0", "}", 
                       "^", "\\inf", "a", "_", "i"};
    ASSERT_EQ(result, expected);
}
