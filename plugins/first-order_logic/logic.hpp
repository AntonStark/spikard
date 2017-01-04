//
// Created by anton on 03.12.16.
//

#ifndef TEST_BUILD_LOGIC_HPP
#define TEST_BUILD_LOGIC_HPP

#include <string>
#include <iostream>
#include <list>
#include <stdexcept>
#include <memory>

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
    Named(const Named& one) : name(one.name) {}
    ~Named() {}

    std::string getName() const { return name; }
    bool operator== (const Named& one) const;
};

class Symbol : public virtual Printable, public Named
{
public:
    Symbol(const std::string& _name) : Named(_name) {}
    Symbol(const Symbol& one) : Named(one) {}
    virtual ~Symbol() {}

    void print(std::ostream& out = std::cout) const override;
    bool operator== (const Symbol& one) const;
};

class Map
{
private:
    const unsigned arity;
public:
    Map(unsigned _arity) : arity(_arity) {}
    Map(const Map& one) : arity(one.arity) {}
    virtual ~Map() {}

    unsigned getArity() const { return arity; }
    bool operator== (const Map& one) const;
};

/*class Terms;
class Term;
class Atom;*/
class Predicate : public Symbol, public Map
{
public:
    Predicate(const std::string& _name, unsigned _arity)
            : Symbol(_name), Map(_arity) {}
    Predicate(const Predicate& one)
            : Symbol(one), Map(one) {}
    virtual ~Predicate() {}

    bool operator== (const Predicate& one) const;

    /*const Atom& operator() (std::vector<Terms*> _args);
    { return Atom::Atom(this, _args); }*/
};

class Function : public Symbol, public Map
{
public:
    Function(const std::string& _name, unsigned _arity)
            : Symbol(_name), Map(_arity) {}
    Function(const Function& one)
            : Symbol(one), Map(one) {}
    virtual ~Function() {}

    bool operator== (const Function& one) const;

    /*const Term& operator() (std::vector<Terms*> _args);
    { return Term::Term(this, _args); }*/
};

class Terms : public virtual Printable
{
public:
    virtual ~Terms() {}
    virtual Terms* clone() const = 0;
};

class Variable : public Terms, public Symbol
{
public:
    Variable(const std::string& _name) : Symbol(_name) {}
    Variable(const Variable& one) : Symbol(one) {}
    virtual ~Variable() {}

    Variable* clone() const override { return (new Variable(*this)); }
};

class Constant : public Terms, public Symbol
{
public:
    Constant(const std::string& _name) : Symbol(_name) {}
    Constant(const Constant& one) : Symbol(one) {}
    virtual ~Constant() {}

    Constant* clone() const override { return (new Constant(*this)); }
};

class ParenSymbol : public virtual Printable
{
public:
    typedef std::list<std::shared_ptr<Terms> > TermsList;
private:
    TermsList args;
protected:
    void argCheck(Map* f, std::list<Terms*> _args);
    void argCheck(std::shared_ptr<Map> f, TermsList _args);
public:
    ParenSymbol(std::list<Terms*> _args)
    {
        for (auto t : _args)
            args.push_back(std::shared_ptr<Terms>(t->clone()));
    }
    ParenSymbol(TermsList _args) : args(_args) {}
    virtual ~ParenSymbol() {}

    virtual void print(std::ostream& out = std::cout) const override;
    class nArg_arity_error;
};

class Term : public Terms, protected Function, protected ParenSymbol
{
public:
    Term(Function* f, std::list<Terms*> _args)
            : Function(*f), ParenSymbol(_args) { argCheck(f, _args); }
    Term(const Term& one)
            : Function(one), ParenSymbol(one) {}
    Term(std::shared_ptr<Function> f, TermsList _args)
            : Function(*f.get()), ParenSymbol(_args) { argCheck(f, _args); }
    virtual ~Term() {}

    Term* clone() const override { return (new Term(*this)); }
    virtual void print(std::ostream& out = std::cout) const override;
};

class Modifier : public virtual Printable
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
};

/*const Atom& Predicate::operator() (std::vector<Terms*> _args)
{ return Atom(this, _args); }
const Term& Function ::operator() (std::vector<Terms*> _args)
{ return Term(this, _args); }*/

#endif //TEST_BUILD_LOGIC_HPP
