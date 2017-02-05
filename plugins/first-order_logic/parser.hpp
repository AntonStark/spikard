//
// Created by anton on 14.01.17.
//

#ifndef TEST_BUILD_INTERPR_HPP
#define TEST_BUILD_INTERPR_HPP

#include "signature.hpp"

//TODO мысли о рефакторинге:
/*-1)-Interpr -> Parser
 * 2) class Lexeme. Lexeme::operator==(Lexer::Token)
 * 3) std::string Lexeme::value() const;
 * 4) Lexeme.second.second = indent -> lexLen
 * 5) addV/getV -> makeVar
 * 6) checkBrackets/fwdPair... -> map<iterator, iterator> pairBracket;
 */

class Lexer
{
public:
    enum class Token {P, F, V,
        Ln, La, Lo, Qf, Qe, Lt,
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

typedef std::pair<Lexer::Token,
                  std::pair<size_t, size_t> > Lexeme;
typedef std::list<Lexeme> LexList;
typedef std::pair<size_t, LexList> PartialResolved;
typedef std::map<LexList::const_iterator,
                 LexList::const_iterator> BracketMap;

class Parser
{
public:
    const Lexer& lex;
    FormulasFactory& ff;
    TermsFactory& tf;
    const Signature& sigma;
    const std::string input;

    /*class Lexeme
    {
    public:
        const Lexer::Token type;
        const std::pair<size_t, size_t> piece;

        Lexeme(const Lexer::Token& _type, std::pair<size_t, size_t> _piece)
                : type(_type), piece(_piece)
        {
            if (piece.second > input.length())
                throw std::invalid_argument("Выход за границу строки.\n");
        }
        Lexeme(const Lexeme& one)
                : type(one.type), piece(one.piece) {}

        ~Lexeme() {}

        std::string value() const
        { return input.substr(piece.first, piece.second - piece.first); }
    };*/


    /*struct Compare
    {
        bool operator()(const std::pair<std::string,
                                        LexList >& a,
                        const std::pair<std::string,
                                        LexList >& b) const
        {
            if (a.first.length() != b.first.length())
                return (a.first.length() > b.first.length());
            else
                return (a.second < b.second);
        }
    };*/

    std::set<PartialResolved/*, Compare*/> stage1;
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

void prepareForName(std::string& _name);
size_t findPairBracket(const std::string& source, size_t pos);
void stripBrackets(std::string& text);
bool splitByTopLevelLO(std::string source, std::string& left, MType& type, std::string& right);


#endif //TEST_BUILD_INTERPR_HPP
