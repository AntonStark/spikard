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
    void defPrint(std::ostream&)
    { out << name; }
public:
    const std::string name;
    Symbol(std::string& _name)
            : name(_name) {}
    virtual ~Symbol() {}

    virtual void print(std::ostream& out = std::cout) const = 0;
    string& getName() const
    { return name; }
};

class Map
{
public:
    const unsigned arity;
    Map(unsigned _arity)
            : arity(_arity) {}
    virtual ~Map() {}
};

class Predicate : public Map, public Symbol
{
public:
    Predicate(std::string& _name, unsigned _arity)
            : Symbol(_name),
              Map(_arity) {}
    Predicate(Predicate& one)
            : Symbol(one.name),
              Map(one.arity) {}

    void print(std::ostream& out) const
    { Symbol::defPrint(out); }

    Atom operator() (std::vector<Terms*> _args)
    { return Atom::Atom(this, _args); }
};

class Function : public Map, public Symbol
{
public:
    Function(std::string& _name, unsigned _arity)
            : Symbol(_name),
              Map(_arity) {}
    Function(Function& one)
            : Symbol(one.name),
              Map(one.arity) {}

    void print(std::ostream& out) const
    { Symbol::defPrint(out); }

    Term operator() (std::vector<Terms*> _args)
    { return Term::Term(this, _args); }
};

class ParenSymbol : public Symbol
{
public:
    std::vector<Terms*> args;

    ParenSymbol(std::vector<Terms*> _args)
            : args(_args) {}

    virtual void print(ostream& out) const override
    {
        out << '(';
        for (unsigned i = 0; i < args.size()-1; ++i)
            out << args[i]->print() << ',';
        if (!args.empty())
            out << args.back()->print();
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

class Term : public Terms, protected Function, protected ParenSymbol
{
public:
    Term(Function* f, std::vector<Terms*> _args)
            : Function(*f),
              ParenSymbol(_args)
    {
        static_assert(_args.size() == arity,
                      "Кол-во аргументов терма не соответствует арности его функционального символа");
    }
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

class Atom : public Formula, protected Predicate, protected ParenSymbol
{
    Atom(Predicate* p, std::vector<Terms*> _args)
            : Predicate(*p),
              ParenSymbol(_args)
    {
        static_assert(_args.size() == arity,
                      "Кол-во аргументов атома не соответствует арности его предикатного символа");
    }
};

#endif //TEST_BUILD_LOGIC_HPP
