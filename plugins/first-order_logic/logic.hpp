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
protected:
    void defPrint(std::ostream& out) const
    { out << name; }
public:
    const std::string name;
    Symbol() {}
    Symbol(const std::string& _name)
            : name(_name) {}
    Symbol(Symbol& one)
            : name(one.name) {}
    virtual ~Symbol() {}

    virtual void print(std::ostream& out = std::cout) const = 0;
    std::string getName() const
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
            : Symbol(one.name),
              Map(one.arity) {}
    virtual ~Predicate() {}

    void print(std::ostream& out = std::cout) const override
    { Symbol::defPrint(out); }

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
            : Symbol(one.name),
              Map(one.arity) {}
    virtual ~Function() {}

    void print(std::ostream& out = std::cout) const override
    { Symbol::defPrint(out); }

    /*const Term& operator() (std::vector<Terms*> _args);
    { return Term::Term(this, _args); }*/
};

class Terms : public Symbol
{
public:
    Terms() {}
    Terms(const std::string& _name)
            : Symbol(_name) {}
    /*virtual void print(std::ostream& out = std::cout) const override
    { Symbol::defPrint(out); }*/
    virtual ~Terms() {}
};

class Variable : public Terms
{
public:
    Variable(const std::string& _name)
            : Terms(_name) {}
    virtual ~Variable() {}
    void print(std::ostream& out = std::cout) const override
    { Symbol::defPrint(out); }
};

class Constant : public Terms
{
public:
    Constant(const std::string& _name)
            : Terms(_name) {}
    virtual ~Constant() {}
    void print(std::ostream& out = std::cout) const override
    { Symbol::defPrint(out); }
};

class ParenSymbol : public Symbol
{
public:
    std::vector<Terms*> args;

    ParenSymbol(std::vector<Terms*> _args)
            : args(_args) {}
    virtual ~ParenSymbol() {}

    virtual void print(std::ostream& out = std::cout) const override
    {
        out << '(';
        for (unsigned i = 0; i < args.size()-1; ++i)
        { args[i]->print(out); out << ", "; }
        if (!args.empty())
            args.back()->print(out);
        out << ')';
    }
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

    virtual void print(std::ostream& out = std::cout) const override
    {
        Function::print(out);
        ParenSymbol::print(out);
    }
};

class Modifier : public Symbol
{
public:
    virtual bool isQuantifier() = 0;
    virtual bool isLOperation() = 0;
    virtual unsigned getType() = 0;
    virtual ~Modifier() {}
};

class LOperation : public Modifier
{
public:
    enum class LType {NOT = 0, AND, OR, THAN};
    LType type;
    LOperation(LOperation::LType _type)
            : Modifier(),
              type(_type) {}
    virtual ~LOperation() {}

    bool isQuantifier() override { return false;}
    bool isLOperation() override { return true;}
    unsigned getType() override { return static_cast<unsigned>(type);}

    void print(std::ostream& out = std::cout) const override
    {
        switch (type)
        {
            case LType::NOT : { out << "\\not"; break; }
            case LType::AND : { out << "\\and"; break; }
            case LType::OR  : { out << "\\or";  break; }
            case LType::THAN: { out << "\\than";break; }
        }
    }
};

class Quantifier : public Modifier
{
public:
    enum class QType {FORALL = 0, EXISTS};
    QType type;
    Variable* arg;
    Quantifier(QType _type, Variable* _arg)
            : Modifier(),
              type(_type), arg(_arg) {}
    virtual ~Quantifier() {}

    bool isQuantifier() override { return true;}
    bool isLOperation() override { return false;}
    unsigned getType() override { return static_cast<unsigned>(type);}

    void print(std::ostream& out = std::cout) const override
    {
        if (type == QType::FORALL)
            out << "\\forall";
        else
            out << "\\exists";
        arg->print(out);
    }
};

class Formula : public Symbol
{
public:
    Formula *arg1, *arg2;
    Modifier* mod;

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

    void print(std::ostream& out = std::cout) const override
    {
        if (!arg1)
            return;
        if (mod)
        {
            if (arg2)
            {
                arg1->print(out);
                mod->print(out);
                mod->print(out);
            }
            else
            {
                mod->print(out);
                arg1->print(out);
            }
        }
        else
        {
            arg1->print(out);
        }
    }
};

class Atom : public Formula, protected Predicate, protected ParenSymbol
{
public:
    Atom(Predicate* p, std::vector<Terms*> _args)
            : Formula(),
              Predicate(*p),
              ParenSymbol(_args)
    {
        arg1 = this;
       /* static_assert(args.size() == arity,
                      "Кол-во аргументов атома не соответствует арности его предикатного символа");*/
    }
    virtual ~Atom() {}

    void print(std::ostream& out = std::cout) const override
    {
        Predicate::print(out);
        ParenSymbol::print(out);
    }
};

/*const Atom& Predicate::operator() (std::vector<Terms*> _args)
{ return Atom(this, _args); }
const Term& Function ::operator() (std::vector<Terms*> _args)
{ return Term(this, _args); }*/

#endif //TEST_BUILD_LOGIC_HPP
