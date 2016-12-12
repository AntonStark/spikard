//
// Created by anton on 03.12.16.
//

#ifndef TEST_BUILD_LOGIC_HPP
#define TEST_BUILD_LOGIC_HPP

#include <string>
#include <ostream>
#include <vector>

class Symbol
{
protected:
    void defPrintf(std::ostream&)
    { out << name; }
public:
    const std::string name;
    Symbol(std::string& _name)
    { name = _name; }
    virtual ~Symbol() {};

    virtual void print(std::ostream& out = std::cout) const = 0;
    string& getName() const
    { return name; }
};

class Map
{
public:
    const unsigned arity;
};

class Predicate : public Map, public Symbol
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class Function : public Map, public Symbol
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

template <typename argsT, unsigned n_args>
class ParenSymbol : public Symbol, public Map
{
public:
    argsT* args[n_args];

    ParenSymbol(std::vector<argsT*> _args)
    {
        arity = n_args;
        static_assert(_args.size() == arity,
                      "Кол-во аргументов в конструкторе ParenSymbol не соответствует его арности");
        for (unsigned i = 0; i < arity; ++i)
            args[i] = _args[i];
    }

    virtual void print(ostream& out) const override
    {
        out << '(';
        for (unsigned i = 0; i < arity; ++i)
        { out << args[i]->print() << ','; }
        out << ')';
    }
};

class Terms : public Symbol
{

};

class Variable : public Terms
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class Constant : public Terms
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class Term : public Terms, protected Function, protected ParenSymbol<Terms, Function::arity>
{
public:
    /*Term(Variable var) :
            Symbol(var.getName()) {}
    Term(Constant c) :
            Symbol(c.getName()) {}
    Term(Function* func, std::vector<Term*> _args) :
            ParenSymbol(func->getName(), _args) {}*/

};


class Modifier : public Symbol
{

};

enum class LOperation : public Modifier
{NOT, AND, OR, THAN};

class Quantifier : public Modifier
{
public:
    Variable* argument;
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class Formula : public Symbol
{
public:
    Formula *arg1, *arg2;
    Modifier* conn;

    Formula(Modifier* oper, Formula arg1, Formula arg2 = nullptr)
            : arg1(arg), conn(oper), arg2(arg2) {}
};

class Atom : public Formula, protected Predicate, protected ParenSymbol<Predicate, Predicate::arity>
{

};

#endif //TEST_BUILD_LOGIC_HPP
