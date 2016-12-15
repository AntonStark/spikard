//
// Created by anton on 03.12.16.
//

#ifndef TEST_BUILD_LOGIC_HPP
#define TEST_BUILD_LOGIC_HPP

#include <string>
#include <iostream>
#include <vector>

class Symbol
{
private:
    const std::string name;
public:
    Symbol() {}
    Symbol(const std::string& _name)
            : name(_name) {}
    Symbol(Symbol& one)
            : name(one.name) {}
    virtual ~Symbol() {}

    virtual void print(std::ostream& out = std::cout) const;
    virtual std::string getName() const; //TODO Использование этого метода при множественном наследовании вызывает неоднозначности
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

class Terms : public Symbol
{
public:
    Terms()
            : Symbol() {}
    Terms(const std::string& _name)
            : Symbol(_name) {}
    virtual ~Terms() {}
};

class Variable : public Terms
{
public:
    Variable(const std::string& _name)
            : Terms(_name) {}
    virtual ~Variable() {}
};

class Constant : public Terms
{
public:
    Constant(const std::string& _name)
            : Terms(_name) {}
    virtual ~Constant() {}
};

class ParenSymbol : public Symbol
{
private:
    std::vector<Terms*> args;
public:
    ParenSymbol(std::vector<Terms*> _args)
            : Symbol(),
              args(_args) {}
    virtual ~ParenSymbol() {}

    virtual void print(std::ostream& out = std::cout) const override;
};

class Term : public Terms, protected Function, protected ParenSymbol
{
public:
    Term(Function* f, std::vector<Terms*> _args)
            : Terms(),
              Function(*f),
              ParenSymbol(_args)
    {
       /* static_assert(args.size() == arity,
                      "Кол-во аргументов терма не соответствует арности его функционального символа");*/
    }
    virtual ~Term() {}

    std::string getName() const override;
    virtual void print(std::ostream& out = std::cout) const override;
};

class Modifier : public virtual Symbol
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
            : Modifier(),
              type(_type) {}
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
            : Modifier(),
              type(_type), arg(_arg) {}
    virtual ~Quantifier() {}

    bool isQuantifier() const override { return true;}
    bool isLOperation() const override { return false;}
    unsigned getType() const override { return static_cast<unsigned>(type);}

    void print(std::ostream& out = std::cout) const override;
};

class Formula : public virtual Symbol
{
private:
    Formula *arg1, *arg2;
    Modifier* mod;
public:
    Formula()
            : arg1(nullptr), mod(nullptr), arg2(nullptr) {}
    Formula(Modifier* _mod, Formula* arg1, Formula* arg2 = nullptr)
            : Symbol(),
              arg1(arg1), mod(_mod), arg2(arg2)
    {
        /*static_assert( (mod->isLOperation() && (mod->getType() == 0)) && arg2 == nullptr,
                       "Отрицание - унарная операция");
        static_assert( (mod->isLOperation() && (mod->getType() != 0)) && arg2 != nullptr,
                       "Только отрицание - унарная операция");
        static_assert(mod->isQuantifier() && arg2 == nullptr,
                      "При добавлении квантора не должно быть второй формулы");*/
    }
    virtual ~Formula() {}

    void print(std::ostream& out = std::cout) const override;
};

class Atom : public Formula, protected Predicate, protected ParenSymbol
{
public:
    Atom(Predicate* p, std::vector<Terms*> _args)
            : Formula(),
              Predicate(*p),
              ParenSymbol(_args)
    {
       /* static_assert(args.size() == arity,
                      "Кол-во аргументов атома не соответствует арности его предикатного символа");*/
    }
    virtual ~Atom() {}

    std::string getName() const override;
    void print(std::ostream& out = std::cout) const override;
};

/*const Atom& Predicate::operator() (std::vector<Terms*> _args)
{ return Atom(this, _args); }
const Term& Function ::operator() (std::vector<Terms*> _args)
{ return Term(this, _args); }*/

#endif //TEST_BUILD_LOGIC_HPP
