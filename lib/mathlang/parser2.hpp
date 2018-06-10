//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_PARSER2_HPP
#define SPIKARD_PARSER2_HPP

#include <string>

namespace Parser2
{
    enum class Token
    {
        N, c, s, lb, rb,    // Name, Comma, Space, Left/RightBracket - в том числе \bigl \Biggl и т.д.
        lc, rc, t, b        // Left/RightCurve {}, Top ^, Bottom _
    };

    class LexList;
    struct Lexeme
    {
        std::string val;
        Token tok;
        LexList *top, *bottom;

        Lexeme(std::pair<const std::string, Token> pair)
            : val(pair.first), tok(pair.second), top(nullptr), bottom(nullptr) {}
        ~Lexeme() { delete top; delete bottom; }
        bool operator<(const Lexeme& one) const
        { return (val != one.val ? val < one.val : tok < one.tok); }
    };
}

#endif //SPIKARD_PARSER2_HPP
