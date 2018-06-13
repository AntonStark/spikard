//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_PARSER2_HPP
#define SPIKARD_PARSER2_HPP

#include <string>

#include "rationale.hpp"

namespace Parser2
{

const char texSpecial[6] = {' ', '\\', '_', '^', '{', '}'};
struct TeXCommand
{
    const std::string _cmd;
    TeXCommand(const std::string& cmd) : _cmd(cmd) {}
};

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

class LexList
{
public:
    std::list<Lexeme> data;
};

class Lexer
{
//private:
public:
    const PrimaryNode* _where;
    Hidden localNames;
    std::map<std::string, Token> words;

    std::string input;
    std::vector<TeXCommand> inputAsCmds;

    void collectWords() {
        for (auto& s : _where->index().getNames(NameTy::SYM))
            words[s] = Token::N;
        for (const auto& s : _where->index().getNames(NameTy::VAR))
            words[s] = Token::N;
        for (const auto& s : _where->index().getNames(NameTy::CONST))
            words[s] = Token::N;
        words[ Term::qword[Term::QType::FORALL] ] = Token::N;
        words[ Term::qword[Term::QType::EXISTS] ] = Token::N;

        words[","] = Token::c;  words[" "] = Token::s;
        words["("] = Token::lb; words[")"] = Token::rb;
        // todo внести также команды TeX для скобок
        words["{"] = Token::lc; words["}"] = Token::rc;
        words["^"] = Token::t;  words["_"] = Token::b;
    }
    void splitToCmds() {
        size_t i = 0;
        size_t j = 0;

        while (i < input.length()) {
            if (input.at(i) == '\\') {  // команда или экранированный символ
                /* Лучше оставить на потом все проверки на TeX-валидность
                 * if (not i+1 < input.length())
                    throw std::invalid_argument("Строка не может оканчиваться на \\.");*/
                j = input.find_first_of(texSpecial, i+1);
                if (j != std::string::npos && j-i == 1)
                    ++j;                // если экранированный символ, захватываем оба
            }
            else
                j = i+1;

            inputAsCmds.emplace_back(input.substr(i, j-i));
            i = j;
        }
    }
public:
    Lexer(PrimaryNode* where) : _where(where), localNames(where) {}
};

}

#endif //SPIKARD_PARSER2_HPP
