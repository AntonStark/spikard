//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_LEXER2_HPP
#define SPIKARD_LEXER2_HPP

#include <string>
#include <utility>
#include <queue>

#include "rationale.hpp"

namespace Parser2
{

enum class Token {w, t, b, l, r,
                    ls, rs, lc, rc};
std::string printToken(const Token& t);

struct Lexeme
{
    Token  _tok;
    size_t _pos;
    size_t _len;

    Lexeme(Token structureTok) : _tok(structureTok), _pos(0), _len(0) {}
    Lexeme(size_t pos, size_t len)
        : _tok(Token::w), _pos(pos), _len(len) {}
    bool operator< (const Lexeme& two) const
    { return (_tok != two._tok ? _tok < two._tok : _pos < two._pos); }
    bool operator== (const Lexeme& other) const
    { return (_tok != Token::w
                ? _tok == other._tok
                : (other._tok == Token::w)
                    && (_pos == other._pos)
                    && (_len == other._len) ); }
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

struct ExpressionLayer
{
    const LexemeSequence& _base;
    const std::pair<size_t, size_t> _bounds;
    std::map<size_t, size_t> _excludes;
    std::map<size_t, size_t> bracketPairs;

    std::pair<ExpressionLayer*, size_t> _parent;

    struct Iter
    {
        const ExpressionLayer& _host;
        size_t pos;
        bool end;

        Iter(const ExpressionLayer& host, size_t i)
            : _host(host), pos(i), end(false) {}
        Iter& operator++ () {
            auto search = _host._excludes.find(pos + 1);
            if (search != _host._excludes.end())
                pos = search->second + 1;
            else
                ++pos;
            end = (pos > _host._bounds.second);
            return *this;
        }
        Iter& operator= (const Iter& other) {
            if (_host._base == other._host._base) {
                pos = other.pos;
                end = other.end;
            }
        }
        Token tok() {
            return ( !end
                     ? _host._base.at(pos)._tok
                     : Token::w );
        }
    };

    ExpressionLayer(const LexemeSequence& base, std::pair<size_t, size_t> bounds,
        ExpressionLayer* parent, size_t indent) : _base(base), _parent({parent, indent}), _bounds(bounds) {}
    ExpressionLayer(const LexemeSequence& base) : ExpressionLayer(base, {0, base.size()-1}, nullptr, 0) {}

    bool operator< (const ExpressionLayer& two) const
    { return (_excludes.size() != two._excludes.size()
              ? _excludes.size() < two._excludes.size()
              : _bounds < two._bounds); }

    ExpressionLayer* insertSublayer(std::pair<size_t, size_t> bounds) {
        size_t spaceN = _excludes.size();
        _excludes.emplace(bounds);
        return new ExpressionLayer(_base, bounds, this, spaceN);
    }

    LexemeSequence getLexems() {
        LexemeSequence buf;
        size_t at = _bounds.first;
        for (const auto& e : _excludes) {
            for (size_t i = at; i < e.first; ++i)
                buf.emplace_back(_base.at(i));
            at = e.second + 1;
        }
        for (size_t i = at; i <= _bounds.second; ++i)
            buf.emplace_back(_base.at(i));
        return buf;
    }

    Iter begin() { return Iter(*this, _bounds.first); }
};

struct CurAnalysisData;
class Lexer
{
public:
    static ParseStatus splitTexUnits(const std::string& input, LexemeSequence& lexems);
    static ParseStatus collectBracketInfo(const LexemeSequence& lexems, std::map<size_t, size_t>& bracketInfo);
    static void buildLayerStructure(CurAnalysisData* data, ExpressionLayer* target);
//    static ParseStatus checkRegisters(ExpressionLayer* layer, ExpressionLayer::Iter& it, size_t bound = 0);
    static ParseStatus checkRegisters(ExpressionLayer* layer);

    // todo не статик соответствие id->word и хранить в Lexem id вместо (_pos, _len)
    /*static std::set<TexSequence> selectSuitableWithIndent(const std::set<TexSequence>& definedTexSeq,
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
    ParseStatus res;

    CurAnalysisData(std::string toParse);

    inline std::string getVal(const Lexeme& l) const
    { return (l._tok == Token::w ? input.substr(l._pos, l._len) : printToken(l._tok)); }
};

CurAnalysisData parse(PrimaryNode* where, std::string toParse);

}

#endif //SPIKARD_LEXER2_HPP
