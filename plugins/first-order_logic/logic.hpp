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
#include <functional>

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
    bool operator< (const Named& other) const;
};

class Label : public virtual Printable, public Named
{
public:
    Label(const std::string& _name) : Named(_name) {}
    virtual ~Label() {}
    Label(const Label& one) : Named(one) {}

    void print(std::ostream& out = std::cout) const override;
    bool operator== (const Label& one) const;
    bool operator< (const Label& other) const;
};

class MathType
{
private:
    std::string type;
public:
    MathType(std::string _type) : type(_type) {}
    MathType(const MathType& one) : type(one.type) {}
    ~MathType() {}
    const MathType& operator=(const MathType& one)
    {
        if (one.type != this->type)
            this->type = one.type;
        return *this;
    }
    bool operator== (const MathType& other) const;
    bool operator!= (const MathType& other) const
    { return (!(this->operator==)(other)); }
    bool operator<(const MathType& other) const;
};
extern MathType natural_mt;
extern MathType logical_mt;
extern MathType set_mt;

// Указание типов в отображнеии вовлекает семантику, но позволяет более полное описание сущности
// Важно, что с априорной информацией о типах упрощается парсер.
// fixme Это реализация неоднородного символа.
// Подходит для символов малой арности, но не каких-нубудь R^n->R^m
class Map
{
private:
    const std::list<MathType> argT;
//    const unsigned arity;
    const MathType retT;
public:
    Map(const std::list<MathType>& _argT, /*unsigned _arity,*/ MathType _retT)
            : argT(_argT), /*arity(_arity),*/ retT(_retT) {}
    virtual ~Map() {}
    Map(const Map& one) : argT(one.argT), /*arity(one.arity),*/ retT(one.retT) {}

    unsigned getArity() const { return /*arity*/argT.size(); }
    bool operator== (const Map& one) const;
    MathType getArgType() const { return argT.front(); }
    bool matchArgType(const std::list<MathType> otherArgT) const
    { return (otherArgT == argT); }
    MathType getRetType() const { return retT; }
    bool operator< (const Map& other) const;
};

/*=====================================================*/
/*=====================================================*/
/*=====================================================*/

class Symbol : public Label, public Map
{
public:
    Symbol(const std::string& _name,
           std::list<MathType> _argT, /*unsigned _arity,*/ MathType _retT)
            : Label(_name), Map(_argT, /*_arity,*/ _retT) {}

    virtual ~Symbol() {}
    Symbol(const Symbol& one)
            : Label(one), Map(one) {}
    bool operator== (const Symbol& one) const;
    typedef std::pair<std::string,
                      std::pair<std::list<MathType>, MathType> > Sign;
    Symbol(const Sign& sign)
            : Symbol(sign.first,
                     sign.second.first, sign.second.second) {}
    bool operator<(const Symbol& other) const;
};

class Terms : public virtual Printable
{
private:
    MathType type;
public:
    Terms(MathType _type) : type(_type) {}
    Terms(const Terms& one) : type(one.type) {}
    virtual ~Terms() {}
    virtual bool isVariable() const { return false; }
    virtual Terms* clone() const = 0;
    MathType getType() const { return type; }
};

class Variable : public Terms, public Label
{
public:
    Variable(const std::string& _name, MathType _type)
            : Terms(_type), Label(_name) {}
    virtual ~Variable() {}
    Variable(const Variable& one) :
            Terms(one), Label(one) {}
    virtual bool isVariable() const override { return true; }
    virtual Variable* clone() const override { return (new Variable(*this)); }
};

class ParenSymbol : public virtual Printable
{
private:
    class nArg_arity_error;

    // Внимание! ParenSymbol владеет своими аргументами,
    // к передаваемым указателям применяется глубокое копирование
    std::list<Terms*> args;
protected:
    std::set<Variable> vars;
    // Это устаревшая версия
    void argCheck(Map f, std::list<std::reference_wrapper<Terms> > _args);
    void argCheck(Map f, std::list<Terms*> _args);
public:
    ParenSymbol(std::list<std::reference_wrapper<Terms> > _args);
    ParenSymbol(const ParenSymbol& one);
    // Применяется глубокое копирование
    ParenSymbol(std::list<Terms*> _args);
    virtual ~ParenSymbol();

    virtual void print(std::ostream& out = std::cout) const override;
};

class Term : public Terms, public Symbol, public ParenSymbol
{
public:
    Term(Symbol f, std::list<std::reference_wrapper<Terms> > _args)
            : Terms(f.getRetType()), Symbol(f),
              ParenSymbol(_args) { argCheck(f, _args); }
    Term(std::pair<Symbol, std::list<std::reference_wrapper<Terms> > > pair)
            : Term(pair.first, pair.second) {}
    Term(const Term& one)
            : Terms(one), Symbol(one), ParenSymbol(one) {}
    Term(Symbol f, std::list<Terms*> _args)
            : Terms(f.getRetType()), Symbol(f),
              ParenSymbol(_args) {argCheck(f, _args); }
    virtual ~Term() {}

    virtual Term* clone() const override { return (new Term(*this)); }
    virtual void print(std::ostream& out = std::cout) const override;
};

class QuantedTerm : virtual public Printable, public Terms
{
public:
    enum class QType {FORALL, EXISTS};
    static std::map<QType, const std::string> word;
private:
    QType type;
    Variable var;
    Terms* term;
public:
    QuantedTerm(QType _type, Variable _var, Term _term) :
            type(_type), var(_var), term(_term.clone()), Terms(logical_mt)
    {
        if (_term.getRetType() != logical_mt)
            throw std::invalid_argument("Квантификация не логического терма.\n");
    }
    QuantedTerm(QType _type, Variable _var, QuantedTerm _term) :
            type(_type), var(_var), term(_term.clone()), Terms(logical_mt) {}
    QuantedTerm(QType _type, Variable _var, Terms* _term) :
            type(_type), var(_var), term(_term->clone()), Terms(logical_mt)
    {
        if (Term* t = static_cast<Term*>(_term))
        {
            if (t->getRetType() != logical_mt)
                throw std::invalid_argument("Квантификация не логического терма.\n");
        }
        else if (QuantedTerm* qt = static_cast<QuantedTerm*>(_term)) {}
        else throw std::invalid_argument("Квантификация неизвестного вида.\n");
    }
    virtual ~QuantedTerm() {}
    virtual QuantedTerm* clone() const override { return (new QuantedTerm(*this)); }
    virtual void print(std::ostream& out = std::cout) const override;
};

#endif //TEST_BUILD_LOGIC_HPP
