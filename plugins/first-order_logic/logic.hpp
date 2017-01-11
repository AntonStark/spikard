//
// Created by anton on 03.12.16.
//

#ifndef TEST_BUILD_LOGIC_HPP
#define TEST_BUILD_LOGIC_HPP

#include <string>
#include <iostream>
#include <list>
#include <stdexcept>
#include <set>
#include <map>

class Printable
{
public:
    virtual ~Printable() {}

    virtual void print(std::ostream& out = std::cout) const = 0;
    friend std::ostream& operator<<(std::ostream& os, const Printable& pr);
};

class Named
{
private:
    const std::string name;
public:
    Named(const std::string& _name) : name(_name) {}
    ~Named() {}
    Named(const Named& one) : name(one.name) {}

    std::string getName() const { return name; }
    bool operator== (const Named& one) const;
};

class Symbol : public virtual Printable, public Named
{
public:
    Symbol(const std::string& _name) : Named(_name) {}
    virtual ~Symbol() {}
    Symbol(const Symbol& one) : Named(one) {}

    void print(std::ostream& out = std::cout) const override;
    bool operator== (const Symbol& one) const;
};

class Map
{
private:
    const unsigned arity;
public:
    Map(unsigned _arity) : arity(_arity) {}
    virtual ~Map() {}
    Map(const Map& one) : arity(one.arity) {}

    unsigned getArity() const { return arity; }
    bool operator== (const Map& one) const;
};

/*=====================================================*/
/*=====================================================*/
/*=====================================================*/

class Predicate : public Symbol, public Map
{
private:
    Predicate(const std::string& _name, unsigned _arity/*, Signature* _sigma = nullptr*/)
            : Symbol(_name), Map(_arity)/*, sigma(_sigma)*/ {}
//    friend void Signature::addP(const std::string& name, unsigned arity);
    friend class Signature;
public:
    virtual ~Predicate() {}
    Predicate(const Predicate& one)
            : Symbol(one), Map(one)/*, sigma(one.sigma)*/ {}

    bool operator== (const Predicate& one) const;
};

class Function : public Symbol, public Map
{
private:
    Function(const std::string& _name, unsigned _arity)
            : Symbol(_name), Map(_arity)
    { if (_arity == 0)
            throw std::invalid_argument("нуль-арные функции запрещены, константы задаются явно."); }
//    friend void Signature::addF(const std::string& name, unsigned arity);
    friend class Signature;
public:
    virtual ~Function() {}
    Function(const Function& one)
            : Symbol(one), Map(one) {}

    bool operator== (const Function& one) const;
};

class Variable;
class Terms : public virtual Printable
{
public:
    virtual ~Terms() {}
    virtual bool isVariable() const { return false; }
};

class Constant : public Terms, public Symbol
{
private:
    Constant(const std::string& _name) : Symbol(_name) {}
//    friend void TermsFactory::addC(const std::string& name);
    friend class TermsFactory;
public:
    virtual ~Constant() {}
    Constant(const Constant& one) : Symbol(one) {}
};

class Variable : public Terms, public Symbol
{
private:
    Variable(const std::string& _name) : Symbol(_name) {/*vars.emplace(this);*/}
//    friend void TermsFactory::addV(const std::string& name);
    friend class TermsFactory;
public:
    virtual ~Variable() {}
    Variable(const Variable& one) : Symbol(one) {}

    bool isVariable() const override { return true; }
};

class ParenSymbol : public virtual Printable
{
private:
    class nArg_arity_error;

    std::list<Terms*> args;
protected:
    std::set<Variable*> vars;
    void argCheck(Map* f, std::list<Terms*> _args);

    ParenSymbol(std::list<Terms*> _args);
    ParenSymbol(const ParenSymbol& one) : args(one.args), vars(one.vars) {}
public:
    virtual ~ParenSymbol() {}

    virtual void print(std::ostream& out = std::cout) const override;
};

class Term : public Terms, protected Function, public ParenSymbol
{
private:
    friend class TermsFactory;
    Term(Function* f, std::list<Terms*> _args)
            : Function(*f), ParenSymbol(_args) { argCheck(f, _args); }
public:
    Term(const Term& one)
            : Function(one), ParenSymbol(one) {}
    virtual ~Term() {}

    virtual void print(std::ostream& out = std::cout) const override;
};

#endif //TEST_BUILD_LOGIC_HPP
