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
 *-2)-class Lexeme. Lexeme::operator==(Lexer::Token)
 *-3)-std::string Lexeme::value() const;
 * 4) Lexeme.second.second = indent -> lexLen (???)
 *-5)-addV/getV -> makeVar
 *-6)-checkBrackets/fwdPair... -> map<iterator, iterator> pairBracket;
 */

bool matchIndented(const std::string& source, const size_t indent, const std::string& word);

class Lexer
{
private:
    Reasoning& closure;
    void registerVars(std::string& source);
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

    Lexer(Reasoning R);

    void refreshWords(NameTy type);

    struct Lexeme
    {
        std::string val;
        Lexer::Token tok;
        Lexeme(std::pair<const std::string, Token> pair)
                : val(pair.first), tok(pair.second) {}
        bool operator<(const Lexeme& one) const
        {
            if (val < one.val)
                return true;
            else if (one.val < val)
                return false;
            else return (tok < one.tok);
        }
    };
    typedef std::list<Lexeme> LexList;
    typedef std::pair<size_t, LexList> PartialResolved;

    std::set<LexList> lastResult;

    void recognize(std::string source);
};

Terms* parseTerms(const Reasoning& reas, Lexer::LexList& list);
bool addStatement(Reasoning&, std::string source);

#endif //TEST_BUILD_INTERPR_HPP
