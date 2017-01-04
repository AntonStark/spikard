//
// Created by anton on 15.12.16.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <stdexcept>
#include <map>
#include <list>
#include <set>

#include "logic.hpp"

class Signature
{
private:
    std::map<std::string, unsigned> R;
    std::map<std::string, unsigned> F;
    std::set<std::string> C;

    class sym_sets_intersection;
    class sym_exists;
public:
    Signature() {}
    Signature(std::list<std::pair<std::string, unsigned> > _R,
              std::list<std::pair<std::string, unsigned> > _F,
              std::list<std::string> _C);
    ~Signature() {}

    bool isPredName(const std::string &name) const
    { return (R.find(name) != R.end()); }
    bool isFuncName(const std::string &name) const
    { return (F.find(name) != F.end()); }
    bool isConsName(const std::string &name) const
    { return (C.find(name) != C.end()); }

    enum class nameT {predicate, function, constant, none};
    nameT checkName(const std::string& name) const;

    unsigned arity(const std::string& name) const;

    void addPred(const std::string& name, unsigned arity);
    void addFunc(const std::string& name, unsigned arity);
    void addConst(const std::string& name);

    std::shared_ptr<Predicate> getPred(const std::string& name) const;
    std::shared_ptr<Function> getFunc(const std::string& name) const;
    std::shared_ptr<Constant> getCons(const std::string& name) const;

    unsigned long maxLength(nameT type) const;
};

/*///возможные следующие квази-лексемы:
enum class Token {quant, loper, arglist, paren, pred, func, cons, space, literal, err};
*//*enum class token {  tok_qu, tok_lo, tok_al, tok_pa,
                    tok_f, tok_p, tok_c, tok_s, tok_l};*//*
///квантор, логическая операция, скобки перечисления, скобки выделения,
///функциональный символ, предикатный символ, константный -, пробел, литерал
Token lexer(std::string& source, std::string& word, Signature& sigma)
{
    if (source.front() == ' ')
    {
        word = source.substr(0, 1);
        source.erase(source.begin());
        return Token::space;
    }
    if (source[0] == '\\')
    {
        if (source[1] == 'f' || source[1] == 'e')
        {
            if ((source.compare(0, 7, "\\forall") == 0) ||
                (source.compare(0, 7, "\\exists") == 0))
            {
                word = source.substr(0, 7);
                source.erase(0, 7);
                return Token::quant;
            }
        }
        else if (source[1] == 'l' || source[1] == 'R')
        {
            if (source.compare(0, 5, "\\lnot") == 0)
            {
                word = source.substr(0, 5);
                source.erase(0, 5);
                return Token::loper;
            }
            if (source.compare(0, 5, "\\land") == 0)
            {
                word = source.substr(0, 5);
                source.erase(0, 5);
                return Token::loper;
            }
            if (source.compare(0, 4, "\\lor") == 0)
            {
                word = source.substr(0, 4);
                source.erase(0, 4);
                return Token::loper;
            }
            if (source.compare(0, 11, "\\Rightarrow") == 0)
            {
                word = source.substr(0, 11);
                source.erase(0, 11);
                return Token::loper;
            }
        }
    }
    if (source.front() == '(')
    {
        unsigned depth = 1, i = 1;
        bool arglist = false;
        while (i < source.length() && depth > 0)
        {
            if (source[i] == ',' && depth == 1)
                arglist = true;
            if (source[i] == '(')
                ++depth;
            else if (source[i] == ')')
                --depth;
            ++i;
        }
        word = source.substr(0, i);
        source.erase(0, i);
        if (depth == 0)
        if (arglist)
            return Token::arglist;
        else
            return Token::paren;
        else
            return Token::err;
    }
    word = checkPred(source, sigma);
    if (word.length() > 0)
    { source.erase(0, word.length()); return Token::pred; }
    word = checkFunc(source, sigma);
    if (word.length() > 0)
    { source.erase(0, word.length()); return Token::func; }
    word = checkCons(source, sigma);
    if (word.length() > 0)
    { source.erase(0, word.length()); return Token::cons; }

    word = source.substr(0, 1);
    source.erase(source.begin());
    return Token::literal;
}*/

class Interpr
{
public:
    const Signature sigma;
    Interpr(const Signature& _sigma)
            : sigma(_sigma) {}

    bool checkForNOT(const std::string& foText);
    bool checkForQuant(const std::string& foText);
    std::string checkPred(const std::string& source);
    std::string checkFunc(const std::string& source);
    std::string checkCons(const std::string& source);
    bool checkFBegin(const std::string& foText);
    bool checkForAtom(const std::string& text, std::string& pred, std::string& paren);

    std::shared_ptr<LOperation> makeLOperation(const std::string& text);
    std::shared_ptr<Quantifier> makeQuantifier(const std::string& text);
    std::shared_ptr<Modifier> makeModifier(const std::string& text);
    std::shared_ptr<Formulas> makeFormulas(const std::string& source);
    std::shared_ptr<Formula> interpretFormula(std::string foText);
    std::list<std::shared_ptr<Terms> > getTermsFromParen(const std::string& paren);
};

void prepareForName(std::string& _name);
size_t findPairBracket(const std::string& source, size_t pos);
void stripBrackets(std::string& text);
bool splitByTopLevelLO(std::string source, std::string& left, LOperation::LType& type, std::string& right);

#endif //TEST_BUILD_SIGNATURE_HPP
