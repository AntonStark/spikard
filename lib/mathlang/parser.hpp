//
// Created by anton on 14.01.17.
//

#ifndef TEST_BUILD_INTERPR_HPP
#define TEST_BUILD_INTERPR_HPP

#include <stack>
#include <unordered_map>

#include "rationale.hpp"

bool matchIndented(const std::string& source, const size_t indent, const std::string& word);
Terms* parse(PrimaryNode* where, std::string source);

class Lexer
{
public:
    enum class Token {S, V, Q, /*T, to,*/
                c, s, lb, rb};
    static std::string tokToStr(const Token& tok);
    Lexer(PrimaryNode* _where);

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
private:
    PrimaryNode* where;
    std::map<std::string, Token> words;
    std::set<LexList> lastResult;

    void registerVar(std::string& source, unsigned indent);
    void registerVars(std::string& source);
    void refreshWords(NameTy type);

    friend Terms* parse(PrimaryNode* where, std::string source);
    void recognize(std::string source);

    // Вообще, это функционал парсера
    // todo дорефакторить, чтобы они были аккуратно интегрированы
    Term* parseQuantedTerm(Lexer::LexList& list);
    Term* parseTerm(Lexer::LexList& list);
    Terms* parseTerms(Lexer::LexList& list);
};

#endif //TEST_BUILD_INTERPR_HPP
