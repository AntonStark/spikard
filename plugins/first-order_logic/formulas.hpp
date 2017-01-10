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

/*class Modifier : public virtual Printable
{
public:
    virtual bool isQuantifier() const = 0;
    virtual bool isLOperation() const = 0;
    virtual unsigned getType() const = 0;
    virtual ~Modifier() {}
    virtual Modifier* clone() const = 0;
};

class LOperation : public Modifier
{
public:
    enum class LType {NOT = 0, AND, OR, THAN};
private:
    LType type;
public:
    LOperation(LOperation::LType _type) : type(_type) {}
    LOperation(const LOperation& one) : type(one.type) {}
    virtual ~LOperation() {}

    bool isQuantifier() const override { return false;}
    bool isLOperation() const override { return true;}
    unsigned getType() const override { return static_cast<unsigned>(type);}

    void print(std::ostream& out = std::cout) const override;
    LOperation* clone() const override { return (new LOperation(*this)); }
};

class Quantifier : public Modifier
{
public:
    enum class QType {FORALL = 0, EXISTS};
private:
    QType type;
    std::shared_ptr<Variable> arg;
public:
    Quantifier(QType _type, const Variable& _arg)
            : type(_type) { arg = std::make_shared<Variable>(_arg); }
    Quantifier(const Quantifier& one) : type(one.type) {arg = one.arg;}
    virtual ~Quantifier() {}

    bool isQuantifier() const override { return true;}
    bool isLOperation() const override { return false;}
    unsigned getType() const override { return static_cast<unsigned>(type);}

    void print(std::ostream& out = std::cout) const override;
    Quantifier* clone() const override { return (new Quantifier(*this)); }
};

class Formulas : public virtual Printable
{
public:
    virtual bool isAtom() const { return false;};
    virtual ~Formulas() {};

    virtual Formulas* clone() const = 0;
};

class Formula : public Formulas
{
private:
    std::shared_ptr<Formulas> arg1, arg2;
    std::shared_ptr<Modifier> mod;
public:
    Formula(const Formula& one)
            : arg1(one.arg1), mod(one.mod), arg2(one.arg2) {}
    Formula(const Modifier& _mod, const Formulas& F);
    Formula(const LOperation& _mod, const Formulas& F1, const Formulas& F2);
    Formula(std::shared_ptr<Modifier> _mod,
            std::shared_ptr<Formulas> F1,
            std::shared_ptr<Formulas> F2 = nullptr)
            : arg1(F1), mod(_mod), arg2(F2) {}
    virtual ~Formula() {}

    void print(std::ostream& out = std::cout) const override;
    Formula* clone() const override { return (new Formula(*this)); }
};

class Atom : public Formulas, protected Predicate, protected ParenSymbol
{
public:
 //TODO _args по умолчанию пустой список
    Atom(Predicate* p, std::list<Terms*> _args)
            : Predicate(*p), ParenSymbol(_args) { argCheck(p, _args); }
    Atom(const Atom& one)
            : Predicate(one), ParenSymbol(one) {}
    Atom(std::shared_ptr<Predicate> p, TermsList _args)
            : Predicate(*p.get()), ParenSymbol(_args) { argCheck(p, _args); }
    virtual ~Atom() {}

    void print(std::ostream& out = std::cout) const override;
    bool isAtom() const override { return true;}
    Atom* clone() const override { return (new Atom(*this)); }
};*/

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
    std::shared_ptr<Formulas> makeFormulas(const std::string& source);
    std::shared_ptr<Formula> interpretFormula(std::string foText);
    std::list<std::shared_ptr<Terms> > getTermsFromParen(const std::string& paren);
};

void prepareForName(std::string& _name);
size_t findPairBracket(const std::string& source, size_t pos);
void stripBrackets(std::string& text);
bool splitByTopLevelLO(std::string source, std::string& left, LOperation::LType& type, std::string& right);
*/

#endif //TEST_BUILD_SIGNATURE_HPP
