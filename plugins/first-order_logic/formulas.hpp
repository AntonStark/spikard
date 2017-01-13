//
// Created by anton on 15.12.16.
//

#ifndef TEST_BUILD_FORMUALAS_HPP
#define TEST_BUILD_FORMUALAS_HPP

#include <stdexcept>
#include <map>
#include <list>
#include <set>

#include "logic.hpp"

class Modifier : public virtual Printable
{
public:
    enum class MType {NOT = 0, AND, OR, THAN, FORALL, EXISTS};
private:
    class no_arg;
    class excess_arg;
    MType type;
    Variable* arg;

    bool isLogical(MType _type) const;
    bool isQuantor(MType _type) const;
public:
    Modifier(MType _type, Variable* _arg = nullptr);
    Modifier(const Modifier& one) : type(one.type), arg(one.arg) {}
    virtual ~Modifier() {}

    void print(std::ostream& out = std::cout) const override;

    bool isLogical() const;
    bool isQuantor() const;
    bool isUnary() const;

//    Modifier* clone() const { return (new Modifier(*this)); }
};

class Formula : public virtual Printable
{
public:
    virtual bool isAtom() const { return false;};
    virtual ~Formula() {};

    //TODO Если конструкторы ComposedF от ссылок всё-таки будут нужны, нужно
    //делать clone через фабричный метод make.
//    virtual Formula* clone() const = 0;
};

class ComposedF : public Formula
{
private:
    Formula *arg1, *arg2;
    Modifier* mod;
    ComposedF(Modifier* _mod, Formula* F1, Formula* F2 = nullptr)
            : arg1(F1), mod(_mod), arg2(F2) {}
    ComposedF(std::pair<Modifier*,
                        std::pair<Formula*, Formula*> > tuple)
            : ComposedF(tuple.first, tuple.second.first, tuple.second.second) {}
    template <typename K, typename V>
    friend class UniqueObjectFactory;
public:
    ComposedF(const ComposedF& one)
            : arg1(one.arg1), mod(one.mod), arg2(one.arg2) {}
    /*ComposedF(const Modifier& _mod, const Formula& F);
    ComposedF(const Modifier& _mod, const Formula& F1, const Formula& F2);*/
    virtual ~ComposedF() {}

    void print(std::ostream& out = std::cout) const override;
//    ComposedF* clone() const override { return (new ComposedF(*this)); }
};

class Atom : public Formula, protected Predicate, protected ParenSymbol
{
private:
    Atom(Predicate* p, std::list<Terms*> _args)
            : Predicate(*p), ParenSymbol(_args) { argCheck(p, _args); }
    Atom(std::pair<Predicate*, std::list<Terms*> > pair)
            : Atom(pair.first, pair.second) {}
    template <typename K, typename V>
    friend class UniqueObjectFactory;
public:
    Atom(const Atom& one)
            : Predicate(one), ParenSymbol(one) {}
    virtual ~Atom() {}

    void print(std::ostream& out = std::cout) const override;
    bool isAtom() const override { return true;}
//    Atom* clone() const override { return (new Atom(*this)); }
};

/*///возможные следующие квази-лексемы:
enum class Token {quant, loper, arglist, paren, pred, func, cons, space, literal, err};
enum class token {  tok_qu, tok_lo, tok_al, tok_pa,
                    tok_f, tok_p, tok_c, tok_s, tok_l};
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
/*

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
    std::shared_ptr<Formula> makeFormulas(const std::string& source);
    std::shared_ptr<ComposedF> interpretFormula(std::string foText);
    std::list<std::shared_ptr<Terms> > getTermsFromParen(const std::string& paren);
};

void prepareForName(std::string& _name);
size_t findPairBracket(const std::string& source, size_t pos);
void stripBrackets(std::string& text);
bool splitByTopLevelLO(std::string source, std::string& left, LOperation::LType& type, std::string& right);
*/

#endif //TEST_BUILD_FORMUALAS_HPP
