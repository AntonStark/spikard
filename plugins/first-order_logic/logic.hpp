//
// Created by anton on 03.12.16.
//

#ifndef TEST_BUILD_LOGIC_HPP
#define TEST_BUILD_LOGIC_HPP

#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>

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
    Named(const std::string& _name)
            : name(_name) {}
    Named(Named& one)
            : name(one.name) {}
    ~Named() {}

    std::string getName() const;
    bool operator== (const Named& one) const;
};

class Symbol : public virtual Printable, public Named
{
public:
    Symbol(const std::string& _name)
            : Named(_name) {}
    Symbol(Symbol& one)
            : Named(one) {}
    virtual ~Symbol() {}

    void print(std::ostream& out = std::cout) const override;
    bool operator== (const Symbol& one) const;
};

class Map
{
private:
    const unsigned arity;
public:
    Map(unsigned _arity)
            : arity(_arity) {}
    Map(Map& one)
            : arity(one.arity) {}
    virtual ~Map() {}

    unsigned getArity() const;
    bool operator== (const Map& one) const;
};

/*class Terms;
class Term;
class Atom;*/
class Predicate : public Symbol, public Map
{
public:
    Predicate(const std::string& _name, unsigned _arity)
            : Symbol(_name),
              Map(_arity) {}
    Predicate(Predicate& one)
            : Symbol(one),
              Map(one) {}
    virtual ~Predicate() {}

    bool operator== (const Predicate& one) const;

    /*const Atom& operator() (std::vector<Terms*> _args);
    { return Atom::Atom(this, _args); }*/
};

class Function : public Symbol, public Map
{
public:
    Function(const std::string& _name, unsigned _arity)
            : Symbol(_name),
              Map(_arity) {}
    Function(Function& one)
            : Symbol(one),
              Map(one) {}
    virtual ~Function() {}

    bool operator== (const Function& one) const;

    /*const Term& operator() (std::vector<Terms*> _args);
    { return Term::Term(this, _args); }*/
};

class Terms : public virtual Printable
{
public:
    virtual ~Terms() {}
};

class Variable : public Terms, public Symbol
{
public:
    Variable(const std::string& _name)
            : Symbol(_name) {}
    virtual ~Variable() {}
};

class Constant : public Terms, public Symbol
{
public:
    Constant(const std::string& _name)
            : Symbol(_name) {}
    virtual ~Constant() {}
};

class ParenSymbol : public virtual Printable
{
private:
    std::vector<Terms*> args;
public:
    ParenSymbol(std::vector<Terms*> _args)
            : args(_args) {}
    virtual ~ParenSymbol() {}

    virtual void print(std::ostream& out = std::cout) const override;

    class nArg_arity_error : public std::invalid_argument
    {
    public:
        nArg_arity_error()
                : std::invalid_argument("Кол-во аргументов не соответствует арности символа.\n") {}
    };
};

class Term : public Terms, protected Function, protected ParenSymbol
{
public:
    Term(Function* f, std::vector<Terms*> _args)
            : Function(*f),
              ParenSymbol(_args)
    {
        if (_args.size() != f->getArity())
            throw nArg_arity_error();
    }
    virtual ~Term() {}

    virtual void print(std::ostream& out = std::cout) const override;
};

class Modifier : public virtual Printable
{
public:
    virtual bool isQuantifier() const = 0;
    virtual bool isLOperation() const = 0;
    virtual unsigned getType() const = 0;
    virtual ~Modifier() {}
};

class LOperation : public Modifier
{
public:
    enum class LType {NOT = 0, AND, OR, THAN};
private:
    const LType type;
public:
    LOperation(LOperation::LType _type)
            : type(_type) {}
    virtual ~LOperation() {}

    bool isQuantifier() const override { return false;}
    bool isLOperation() const override { return true;}
    unsigned getType() const override { return static_cast<unsigned>(type);}

    void print(std::ostream& out = std::cout) const override;
};

class Quantifier : public Modifier
{
public:
    enum class QType {FORALL = 0, EXISTS};
private:
    const QType type;
    const Variable* arg;
public:
    Quantifier(QType _type, Variable* _arg)
            : type(_type), arg(_arg) {}
    virtual ~Quantifier() {}

    bool isQuantifier() const override { return true;}
    bool isLOperation() const override { return false;}
    unsigned getType() const override { return static_cast<unsigned>(type);}

    void print(std::ostream& out = std::cout) const override;
};

class Formula : public virtual Printable
{
private:
    Formula *arg1, *arg2;
    Modifier* mod;
public:
    Formula() : arg1(nullptr), mod(nullptr), arg2(nullptr) {}
    Formula(Modifier* _mod, Formula* arg1, Formula* arg2 = nullptr)
            : arg1(arg1), mod(_mod), arg2(arg2)
    {
        if ( (mod->isLOperation() && (mod->getType() == 0)) && arg2 != nullptr)
            throw std::invalid_argument("Отрицание - унарная операция.\n");
        if ( (mod->isLOperation() && (mod->getType() != 0)) && arg2 == nullptr)
            throw std::invalid_argument("Только отрицание - унарная операция.\n");
        if (mod->isQuantifier() && arg2 != nullptr)
            throw std::invalid_argument("При добавлении квантора не должно быть второй формулы.\n");
    }
    Formula (const std::string& foText);
    virtual ~Formula() {}

    void print(std::ostream& out = std::cout) const override;
};

class Atom : public Formula, protected Predicate, protected ParenSymbol
{
public:
    Atom(Predicate* p, std::vector<Terms*> _args)
            : Predicate(*p),
              ParenSymbol(_args)
    {
        if(_args.size() != p->getArity())
            throw nArg_arity_error();
    }
    virtual ~Atom() {}

    void print(std::ostream& out = std::cout) const override;
};

/*const Atom& Predicate::operator() (std::vector<Terms*> _args)
{ return Atom(this, _args); }
const Term& Function ::operator() (std::vector<Terms*> _args)
{ return Term(this, _args); }*/

#endif //TEST_BUILD_LOGIC_HPP
