//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_PARSER2_HPP
#define SPIKARD_PARSER2_HPP

#include <string>

#include "rationale.hpp"

namespace Parser2
{

//const char texSpecial[6] = {' ', '\\', '_', '^', '{', '}'};
struct TeXCommand
{
    const std::string _cmd;

    TeXCommand(const char cmd[]) : _cmd(cmd) {}
    TeXCommand(const std::string& cmd) : _cmd(cmd) {}
    bool operator< (const TeXCommand& two) const
    { return (_cmd < two._cmd); }
    bool operator== (const TeXCommand& two) const
    { return (_cmd == two._cmd); }
};

extern std::set<TeXCommand> texBrackets;
extern std::map<TeXCommand, TeXCommand>  pairBrackets;

enum class Token
{
    N, c, s, lb, rb,    // Name, Comma, Space, Left/RightBracket - в том числе \bigl \Biggl и т.д.
    lc, rc, t, b        // Left/RightCurly {}, Top ^, Bottom _
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
    std::map<size_t, size_t> bracketInfo;

    void collectWords();

    void splitToCmds();

    std::pair<size_t, std::string> checkForTexErrors();

    size_t findFirstBracketFrom(size_t pos);
    bool isOpenBracket(TeXCommand cmd);
    std::pair<size_t, std::string> findBracketPairs();
public:
    Lexer(PrimaryNode* where) : _where(where), localNames(where) { }
};

}

#endif //SPIKARD_PARSER2_HPP
