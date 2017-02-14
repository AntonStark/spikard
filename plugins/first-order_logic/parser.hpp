//
// Created by anton on 14.01.17.
//

#ifndef TEST_BUILD_INTERPR_HPP
#define TEST_BUILD_INTERPR_HPP

#include <stack>

#include "signature.hpp"

//TODO мысли о рефакторинге:
/*-1)-Interpr -> Parser
 * 2) class Lexeme. Lexeme::operator==(Lexer::Token)
 * 3) std::string Lexeme::value() const;
 * 4) Lexeme.second.second = indent -> lexLen
 * 5) addV/getV -> makeVar
 * 6) checkBrackets/fwdPair... -> map<iterator, iterator> pairBracket;
 */

bool matchIndented(const std::string& source, const size_t indent, const std::string& word);

class Lexer
{
public:
    enum class Token {P, F, V,
        Ln, La, Lo, Lt, Qf, Qe,
        c, s, lb, rb};
static std::string tokToStr(const Token& tok)
{
    switch (tok)
    {
        case Token::P : return "P";
        case Token::F : return "F";
        case Token::V : return "V";
        case Token::Ln: return "Ln";
        case Token::La: return "La";
        case Token::Lo: return "Lo";
        case Token::Qf: return "Qf";
        case Token::Qe: return "Qe";
        case Token::Lt: return "Lt";
        case Token::c : return "c";
        case Token::s : return "s";
        case Token::lb: return "lb";
        case Token::rb: return "rb";
    }
}
    std::map<std::string, Token> words;

    Lexer(const Signature& sigma)
    {
        const Namespace& ns = sigma.viewNS();

        for (auto s : ns.names.at(NameTy::PRED))
            words[s] = Token::P;
        for (auto s : ns.names.at(NameTy::FUNC))
            words[s] = Token::F;

        words[Modifier::word[MType::NOT]]   = Token::Ln;
        words[Modifier::word[MType::AND]]   = Token::La;
        words[Modifier::word[MType::OR]]    = Token::Lo;
        words[Modifier::word[MType::THAN]]  = Token::Lt;
        words[Modifier::word[MType::FORALL]]= Token::Qf;
        words[Modifier::word[MType::EXISTS]]= Token::Qe;

        words[","] = Token::c;  words[" "] = Token::s;
        words["("] = Token::lb; words[")"] = Token::rb;
    }
};

/*bool mayFollow(const Lexer::Token& one, const Lexer::Token& two);

class LexTree
{
public:
    const Lexer* lexer;
    const std::string& input;

    unsigned indent;
    Lexer::Token rootT;
    unsigned length;

    std::list<LexTree*> subs;

    LexTree(const Lexer* _lexer, const std::string& _input, unsigned _indent, Lexer::Token _rootT, unsigned _length)
            : lexer(_lexer), input(_input), indent(_indent), rootT(_rootT), length(_length)
    {
        for (auto w : lexer->words)
        {
            if (mayFollow(rootT, w.second) && matchIndented(input, indent+length, w.first))
                subs.push_back(new LexTree(lexer, input, indent+length, w.second, w.first.length()));
        }
    }
    ~LexTree()
    {
        for (auto s : subs)
            delete s;
    }
};*/

/*class Lexeme
{
public:
    const Lexer::Token type;
    const std::pair<size_t, size_t> piece;
    const std::string& source;

    Lexeme(const Lexer::Token& _type, std::pair<size_t, size_t> _piece, const std::string& _source)
            : type(_type), piece(_piece), source(_source)
    {
        if (piece.second > source.length())
            throw std::invalid_argument("Выход за границу строки.\n");
    }
    Lexeme(const Lexeme& one)
            : type(one.type), piece(one.piece), source(one.source) {}

    ~Lexeme() {}

    std::string value() const
    { return source.substr(piece.first, piece.second - piece.first); }
};*/

typedef std::pair<Lexer::Token,
                  std::pair<size_t, size_t> > Lexeme;
typedef std::list<Lexeme> LexList;
typedef std::pair<size_t, LexList> PartialResolved;

struct LlCiterCompare
{
    bool operator()(const LexList::const_iterator& a,
                    const LexList::const_iterator& b) const
    { return (std::distance(a, b) > 0); }
};
typedef std::map<LexList::const_iterator,
                 LexList::const_iterator, LlCiterCompare> BracketMap;

class Parser
{
public:
    const Lexer& lex;
    FormulasFactory& ff;
    TermsFactory& tf;
    const Signature& sigma;
    const std::string input;

    std::set<PartialResolved> stage1;
    std::set<LexList> stage2;
    std::set<Formula*> stage3;

    Parser(const Lexer& _lex, FormulasFactory& _ff, TermsFactory& _tf,
            Signature& _sigma, const std::string& _input)
            : lex(_lex), ff(_ff), tf(_tf), sigma(_sigma), input(_input)
    {
        markLexems();
        for (auto p : stage1)
        {
            mergeVarPieces(p.second);
            delSpaces(p.second);
            stage2.insert(p.second);
        }
        for (auto vari : stage2)
            stage3.insert(recogniseFormula(vari));
    }

    void markLexems();
    void mergeVarPieces(LexList& list);
    void delSpaces(LexList& list);
    std::string lexToStr(const Lexeme& lex);
    Term* recognizeTerm(LexList::const_iterator& it, LexList::const_iterator end);
    void recognizeParenSymbol(LexList::const_iterator& it, LexList::const_iterator end, std::list<Terms*>& dst);
    Formula* recogniseFormula(const LexList& list);
};

#endif //TEST_BUILD_INTERPR_HPP
