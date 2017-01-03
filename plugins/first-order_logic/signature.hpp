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
public:
    std::map<std::string, unsigned> R;
    std::map<std::string, unsigned> F;
    std::set<std::string> C;

    class sym_sets_intersection : public std::invalid_argument
    {
    public:
        sym_sets_intersection(const std::string& symName)
                : std::invalid_argument("Пересечение множеств символов в конструкторе Signature по символу " + symName +".\n") {}
    };
    class sym_exists : public std::invalid_argument
    {
    public:
        sym_exists(const std::string& symName)
                : std::invalid_argument("Символ с именем \"" + symName + "\" уже определён.\n") {}
    };

    Signature() {}
    Signature(std::list<std::pair<std::string, unsigned> > _R,
              std::list<std::pair<std::string, unsigned> > _F,
              std::list<std::string> _C)
            : R(_R.begin(), _R.end()), F(_F.begin(), _F.end()), C(_C.begin(), _C.end())
    {
        for (auto f : F)
            if (R.find(f.first) != R.end())
                throw sym_sets_intersection(f.first);
        for (auto c : C)
            if (R.find(c) != R.end())
                throw sym_sets_intersection(c);
        for (auto c : C)
            if (F.find(c) != F.end())
                throw sym_sets_intersection(c);
    }
    ~Signature() {}

    bool isPredName(const std::string &name) const
    { return (R.find(name) != R.end()); }
    bool isFuncName(const std::string &name) const
    { return (F.find(name) != F.end()); }
    bool isConsName(const std::string &name) const
    { return (C.find(name) != C.end()); }

    enum class nameT {predicate, function, constant, none};
    nameT checkName(const std::string& name) const
    {
        if (isPredName(name))
            return nameT::predicate;
        if (isFuncName(name))
            return nameT::function;
        if (isConsName(name))
            return nameT::constant;
        return nameT::none;
    }

    unsigned arity(const std::string& name) const
    {
        if (isPredName(name))
        { return R.at(name); }
        else if (isFuncName(name))
        { return F.at(name); }
        else
        { return static_cast<unsigned>(-1);}
    }

    void addPred(const std::string& name, unsigned arity)
    {
        if (checkName(name) != nameT::none)
            throw sym_exists(name);
        R.insert(std::make_pair(name, arity));
    }
    void addFunc(const std::string& name, unsigned arity)
    {
        if (checkName(name) != nameT::none)
            throw sym_exists(name);
        F.insert(std::make_pair(name, arity));
    }
    void addConst(const std::string& name)
    {
        if (checkName(name) != nameT::none)
            throw sym_exists(name);
        C.insert(name);
    }

    std::shared_ptr<Predicate> getPred(const std::string& name) const
    {
        if (isPredName(name))
            return (std::make_shared<Predicate>(name, arity(name)) );
        else
            throw std::invalid_argument("Предикат \"" + name + "\" не определён.\n");
    }
    std::shared_ptr<Function> getFunc(const std::string& name) const
    {
        if (isFuncName(name))
            return (std::make_shared<Function>(name, arity(name)) );
        else
            throw std::invalid_argument("Функиональный символ \"" + name + "\" не определён.\n");
    }
    std::shared_ptr<Constant> getCons(const std::string& name) const
    {
        if (isConsName(name))
            return (std::make_shared<Constant>(name) );
        else
            throw std::invalid_argument("Константа \"" + name + "\" не определена.\n");
    }

    unsigned long maxLength(nameT type) const
    {
        unsigned long maxLen = 0;
        switch (type)
        {
            case nameT::predicate:
            {
                for (auto r : R)
                    if (r.first.length() > maxLen)
                        maxLen = r.first.length();
                break;
            }
            case nameT::function:
            {
                for (auto f : F)
                    if (f.first.length() > maxLen)
                        maxLen = f.first.length();
                break;
            }
            case nameT::constant:
            {
                for (auto c : C)
                    if (c.length() > maxLen)
                        maxLen = c.length();
                break;
            }
            case nameT::none: {}
        }
        return maxLen;
    }
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
    std::vector<std::shared_ptr<Terms> > getTermsFromParen(const std::string& paren);
};

void prepareForName(std::string& _name);
size_t findPairBracket(const std::string& source, size_t pos);
void stripBrackets(std::string& text);
bool splitByTopLevelLO(std::string source, std::string& left, LOperation::LType& type, std::string& right);

#endif //TEST_BUILD_SIGNATURE_HPP
