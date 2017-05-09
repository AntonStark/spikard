//
// Created by anton on 14.01.17.
//

#ifndef TEST_BUILD_INTERPR_HPP
#define TEST_BUILD_INTERPR_HPP

#include <stack>
#include <unordered_map>

#include "signature.hpp"

//TODO мысли о рефакторинге:
/*-1)-Interpr -> Parser
 * 2) class Lexeme. Lexeme::operator==(Lexer::Token)
 * 3) std::string Lexeme::value() const;
 * 4) Lexeme.second.second = indent -> lexLen
 *-5)-addV/getV -> makeVar
 *-6)-checkBrackets/fwdPair... -> map<iterator, iterator> pairBracket;
 */

bool matchIndented(const std::string& source, const size_t indent, const std::string& word);

class Lexer
{
public:
    enum class Token {S, V, Q, /*T, to,*/
                      c, s, lb, rb};
static std::string tokToStr(const Token& tok)
{
    switch (tok)
    {
        case Token::S : return "S";
        case Token::V : return "V";
        case Token::Q : return "Q";
        /*case Token::T : return "T";
        case Token::to: return "to";*/
        case Token::c : return "c";
        case Token::s : return "s";
        case Token::lb: return "lb";
        case Token::rb: return "rb";
    }
}
    std::map<std::string, Token> words;

    Lexer(const Reasoning& R)
    {
        std::set<std::string> buf;
        R.viewSetOfNames(buf, NameTy::SYM);
        for (auto s : buf)
            words[s] = Token::S;
        buf.clear();
        R.viewSetOfNames(buf, NameTy::VAR);
        for (auto s : buf)
            words[s] = Token::V;
        buf.clear();
        /*R.viewSetOfNames(buf, NameTy::MT);
        for (auto s : buf)
            words[s] = Token::T;*/

        words[QuantedTerm::word[QuantedTerm::QType::FORALL]]= Token::Q;
        words[QuantedTerm::word[QuantedTerm::QType::EXISTS]]= Token::Q;
        words[","] = Token::c;  words[" "] = Token::s;
        words["("] = Token::lb; words[")"] = Token::rb;
    }

    typedef std::pair<std::string, Lexer::Token> Lexeme;
    typedef std::list<Lexeme> LexList;
    typedef std::pair<size_t, LexList> PartialResolved;

    std::set<LexList> lastResult;

    void delSpaces(LexList& list);
    void recognize(const std::string& source);
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
    const std::string value;

    Lexeme(const Lexer::Token& _type, const std::string& _value)
            : type(_type), value(_value) {}
    Lexeme(const Lexeme& one)
            : type(one.type), value(one.value) {}
    ~Lexeme() {}
};*/
/*

typedef std::pair<Lexer::Token,
                  std::pair<size_t, size_t> > Lexeme;
typedef std::list<Lexeme> LexList;
typedef std::pair<size_t, LexList> PartialResolved;

typedef LexList::const_iterator Llciter;
struct LlciterCompare
{
    bool operator()(const Llciter& a, const Llciter& b) const
    { return (std::distance(a, b) > 0)*/
/*true*//*
; }
};
struct LLciterHash
{
    size_t operator()(const Llciter& x) const
    { return x->second.first; }
};
//typedef std::map<Llciter, Llciter, LlciterCompare> BracketMap;
typedef std::unordered_map<Llciter, Llciter, LLciterHash> BracketMap;

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
    std::set<const Formula*> stage3;

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
    const Formula* recogniseFormula(const LexList& list);
};
*/

MathType parseType(Reasoning&, std::string&, unsigned&);
void registerVar(Reasoning&, std::string&, unsigned);
void registerVars(Reasoning& closure, std::string& source);
Terms* parseTerm(const Reasoning& reas, Lexer::LexList& list);
void addStatement(Reasoning&, std::string source);

#endif //TEST_BUILD_INTERPR_HPP
