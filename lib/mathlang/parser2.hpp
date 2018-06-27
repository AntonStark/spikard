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
    TeXCommand(const TeXCommand& one) = default;

    bool operator< (const TeXCommand& two) const
    { return (_cmd < two._cmd); }
    bool operator== (const TeXCommand& two) const
    { return (_cmd == two._cmd); }
};

extern std::set<TeXCommand> texBrackets;
extern std::map<TeXCommand, TeXCommand>  pairBrackets;

struct ExpressionLayer
{
    std::vector<TeXCommand> _cmds;
    std::pair<ExpressionLayer*, size_t> _parent;
    unsigned _placeholders;

    ExpressionLayer(ExpressionLayer* parent, size_t indent)
        : _cmds(), _parent({parent, indent}), _placeholders(0) {}

    bool operator< (const ExpressionLayer& two) const
    { return (_placeholders != two._placeholders
              ? _placeholders < two._placeholders
              : _cmds < two._cmds); }
    void insertPlaceholder() {
        _cmds.emplace_back("");
        ++_placeholders;
    }
};

/*enum class Token
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
};*/

class Lexer
{
//private:
public:
    const PrimaryNode* _where;
    Hidden localNames;
//    std::map<std::string, Token> words;

    std::string input;
    std::vector<TeXCommand> inputAsCmds;
    std::map<size_t, size_t> bracketInfo;
    struct comp_by_val {
        bool operator() (ExpressionLayer* const& one,
                         ExpressionLayer* const& two) const
        { return (*one < *two); }
    };
    std::set<ExpressionLayer*, comp_by_val> layers;

//    void collectWords();

    void splitToCmds();

    size_t findFirstBracketFrom(size_t pos);
    bool isOpenBracket(TeXCommand cmd);
    std::pair<size_t, std::string> findBracketPairs();

    std::pair<size_t, std::string> checkForTexErrors();

    void copyCmds(std::vector<TeXCommand>& target, size_t begin, size_t end);
    void buildLayerStructure(ExpressionLayer* parent, size_t i, size_t bound);
public:
    Lexer(PrimaryNode* where) : _where(where), localNames(where) { }
};

}

#endif //SPIKARD_PARSER2_HPP
