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
public:
    Symbol(std::string& _name)
    { name = _name; }
    virtual ~Symbol() {};

    virtual void print(std::ostream& out) const = 0;
    string& getName() const
    { return name; }
};

class Map : public Symbol
{
public:
    const unsigned arity;
};

class Predicate : public Map
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class Function : public Map
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class Variable : public Symbol
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class Constant : public Symbol
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};
//TODO возможно следует определить Variable и Constant как потомков
// Term, чтобы использовать указатели на них единообразно с указателями на Term, а не городить конструкторы приведения

class LOperation : public Symbol
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class Quantifier : public Symbol
{
public:
    void print(std::ostream& out) const
    { Symbol::defPrintf(out); }
};

class ParenSymbol : public Map
{
//protected:
public:

    Symbol* args[arity];

    ParenSymbol(string _name, std::vector<Symbol*> _args) :
            Symbol(_name)
    {
        arity = _args.size();
        for (unsigned i = 0; i < arity; ++i)
            args[i] = _args[i];
    }

    virtual void print(ostream& out) const override
    {
        Symbol::print(out);
        out << '(';
        for (unsigned i = 0; i < arity; ++i)
        { out << args[i]->print() << ','; }
        out << ')';
    }
};

class Term : public Symbol
{
public:
    Term(Variable var) :
            Symbol(var.getName()) {}
    Term(Constant c) :
            Symbol(c.getName()) {}
    Term(Function* func, std::vector<Term*> _args) :
            ParenSymbol(func->getName(), _args) {}
};

#endif //TEST_BUILD_LOGIC_HPP
