//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_LEXER_HPP
#define SPIKARD_LEXER_HPP

#include <string>
#include <utility>
#include <queue>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <sstream>

#include "lexeme.hpp"

namespace Parser2
{

/// Слой выражения представляется набором границ интервалов
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

    bool operator< (const ExpressionLayer& two) const;

    ExpressionLayer* insertSublayer(std::pair<size_t, size_t> bounds);
    LexemeSequence getLexems();
    Iter begin() { return Iter(*this, _bounds.first); }
};

struct ParseStatus
{
    bool success;
    size_t at;
    std::string mess;
    ParseStatus() : success(true), at(size_t(-1)), mess("") {}
    ParseStatus(size_t at, std::string mess)
        : success(false), at(at), mess(std::move(mess)) {}
};

/// Контейнер для вспомогательной информации и
/// промежуточных результатов разбора выражения
struct CurAnalysisData
{
    std::string input;
    LexemeSequence lexems;
    bool blankFound;
    std::map<size_t, size_t> bracketInfo;

    template <typename T>
    struct comp_by_val {
        bool operator() (T* const& one, T* const& two) const
        { return (*one < *two); }
    };
    std::set<ExpressionLayer*, comp_by_val<ExpressionLayer> > layers;
    ParseStatus res;
};

class Lexer
{
    std::map<std::string, Token> structureSymbols;
    std::map<Token, std::string> tokenPrints;
public:
    LexemStorage storage;
    Lexer() {}
    static Lexer configureLatex();
    CurAnalysisData recognize(const std::string& toParse);
    inline std::string print(const Lexeme& l) const
    { return (l._tok == Token::w ? storage.get(l._id) : tokenPrints.at(l._tok)); }
    std::string print(const LexemeSequence& lSeq) const;

    ParseStatus splitTexUnits(const std::string& input, LexemeSequence& lexems);
    void filterNotPtintableCmds(const LexemeSequence& lexems);
    bool hasBlanks(const LexemeSequence& lexems);
    void dropBlanks(LexemeSequence& lexems);
    ParseStatus collectBracketInfo(const LexemeSequence& lexems, std::map<size_t, size_t>& bracketInfo);
    static void buildLayerStructure(CurAnalysisData* data, ExpressionLayer* target);
    static ParseStatus checkRegisters(ExpressionLayer* layer);

};

extern Lexer texLexer;

}

#endif //SPIKARD_LEXER_HPP
