#include <utility>

//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_PARSER2_HPP
#define SPIKARD_PARSER2_HPP

#include <string>
#include <utility>
#include <queue>

#include "rationale.hpp"

namespace Parser2
{

enum class Token {w, t, b, c, s,
                  l, r, ls, rs, lc, rc};
std::string printToken(const Token& t);

struct Lexeme
{
    Token  _tok;
    size_t _pos;
    size_t _len;

    Lexeme(Token tokNotW) : _tok(tokNotW), _pos(0), _len(0) {}
    Lexeme(size_t pos, size_t len)
        : _tok(Token::w), _pos(pos), _len(len) {}
};

typedef std::vector<Lexeme> LexemeSequence;
struct ParseStatus
{
    bool success;
    size_t at;
    std::string mess;
    ParseStatus() : success(true), at(size_t(-1)), mess("") {}
    ParseStatus(size_t at, std::string mess)
        : success(false), at(at), mess(std::move(mess)) {}
};

extern std::set<std::string> blankCommands;
extern std::set<std::string> bracketSizeCommands;
extern std::map<std::string, Token> structureSymbols;
extern std::map<Token, std::string> tokenPrints;
extern std::set<char> skippingChars;

struct PartialResolved
{
    typedef std::vector<LexemeSequence> result_type;

    size_t indent;
    result_type recognized;

    PartialResolved(size_t indent, result_type recognized)
        : indent(indent), recognized(std::move(recognized)) {}
    bool operator< (const PartialResolved& two) const
    { return (indent < two.indent); }
};

struct ExpressionLayer
{
    LexemeSequence _cmds;
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
    void emplaceBack(const LexemeSequence& from, size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i)
            _cmds.emplace_back(from.at(i));
    }
};

struct CurAnalysisData;

class Lexer
{
public:
    const PrimaryNode* _where;
    Hidden localNames; //fixme тут что-то странное: в лексере имена местных переменных не содержатся
    std::set<std::string> namesDefined;
    std::set<LexemeSequence> definedTexSeq;

    Lexer(PrimaryNode* where);
    static ParseStatus splitTexUnits(const std::string& input, LexemeSequence& lexems);
    static ParseStatus collectBracketInfo(const LexemeSequence& lexems, std::map<size_t, size_t>& bracketInfo);

    static void buildLayerStructure(CurAnalysisData* data, ExpressionLayer* parent, size_t pos, size_t bound);

    /*static TexSequence readOneSymbolsCommands(CurAnalysisData* data, size_t from);
    static std::set<TexSequence> selectSuitableWithIndent(const std::set<TexSequence>& definedTexSeq,
                                                          size_t indent, const TexSequence& source);*/
//    static void parseNames(CurAnalysisData* data);

};

/// Контейнер для вспомогательной информации и
/// промежуточных результатов разбора выражения
struct CurAnalysisData
{
    std::string input;
    LexemeSequence lexems;
    std::map<size_t, size_t> bracketInfo;

    template <typename T>
    struct comp_by_val {
        bool operator() (T* const& one, T* const& two) const
        { return (*one < *two); }
    };
    std::set<ExpressionLayer*, comp_by_val<ExpressionLayer> > layers;


    CurAnalysisData(std::string toParse);

    inline std::string getVal(Lexeme& l) const
    { return (l._tok == Token::w ? input.substr(l._pos, l._len) : printToken(l._tok)); }
};

CurAnalysisData parse(PrimaryNode* where, std::string toParse);

}

#endif //SPIKARD_PARSER2_HPP
