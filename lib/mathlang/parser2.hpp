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

enum class Token {w, t, b, l, r,
                    ls, rs, lc, rc};
std::string printToken(const Token& t);

struct Lexeme
{
    Token  _tok;
    size_t _pos;
    size_t _len;
    const std::string& _base;
    std::string val;

    Lexeme(const std::string& source, Token structureTok) : _tok(structureTok), _pos(0), _len(0),
        _base(source), val(_base.substr(_pos, _len)) {} //fimxe debug only
    Lexeme(const std::string& source, size_t pos, size_t len)
        : _tok(Token::w), _pos(pos), _len(len), _base(source), val(_base.substr(_pos, _len)) {}
    bool operator< (const Lexeme& two) const
    { return (_tok != two._tok ? _tok < two._tok : _pos < two._pos); }
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
    const LexemeSequence& _base;    // можно вообще обойтись без копирования, если хранить список включённых интервалов
    LexemeSequence lexems;          // исходной последовательности, но пока неизвестно будет ли это удобно
    std::pair<ExpressionLayer*, size_t> _parent;
    unsigned _placeholders;
    std::map<size_t, size_t> symbolBounds;

    ExpressionLayer(const LexemeSequence& base, ExpressionLayer* parent, size_t indent)
        : _base(base), _parent({parent, indent}), lexems(), _placeholders(0) {}
    ExpressionLayer(const LexemeSequence& base) : ExpressionLayer(base, nullptr, 0) {}

    bool operator< (const ExpressionLayer& two) const
    { return (_placeholders != two._placeholders
              ? _placeholders < two._placeholders
              : lexems < two.lexems); }

    ExpressionLayer* insertPlaceholder() {
        size_t indent = lexems.size();
        lexems.emplace_back("sdfsd", Token::w);
        ++_placeholders;
        return new ExpressionLayer(_base, this, indent);
    }

    void emplaceBack(size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i)
            lexems.emplace_back(_base.at(i));
    }
};

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
    static void buildLayerStructure(CurAnalysisData* data,
        std::pair<size_t, size_t> enclosingBrackets = {0, 0}, ExpressionLayer* target = nullptr);
//    static ParseStatus detectSymbolBounds(ExpressionLayer* layer);

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


    CurAnalysisData(std::string toParse);

    inline std::string getVal(const Lexeme& l) const
    { return (l._tok == Token::w ? input.substr(l._pos, l._len) : printToken(l._tok)); }
};

CurAnalysisData parse(PrimaryNode* where, std::string toParse);

}

#endif //SPIKARD_PARSER2_HPP
