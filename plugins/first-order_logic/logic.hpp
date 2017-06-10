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
#include <vector>

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
    virtual ~Named() {}
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

class Theory;
class MathType : public Named
{
private:
    Theory* definition;
public:
    MathType(std::string _type, Theory* _def = nullptr)
            : Named(_type), definition(_def) {}
    virtual ~MathType() {}
    MathType(const MathType& one)
            : Named(one), definition(one.definition) {}

    //fixme сравнение пока без учета definition
    bool operator== (const MathType& other) const;
    bool operator!= (const MathType& other) const
    { return (!(this->operator==)(other)); }
    bool operator<(const MathType& other) const;
};
extern MathType logical_mt;

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
    MathType getType() const { return retT; }
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
    virtual bool doCompare(const Terms* other) const = 0;
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

    virtual bool doCompare(const Terms* other) const override
    {
        if (const Variable* v = dynamic_cast<const Variable*>(other))
            return (this->Label::operator==)(*v);
        else
            return false;
    }
};

class ParenSymbol : public virtual Printable
{
private:
    class argN_argType_error;

    // Внимание! ParenSymbol владеет своими аргументами,
    // к передаваемым указателям применяется глубокое копирование
    std::vector<Terms*> args;
protected:
    std::set<Variable> vars;
    // Это устаревшая версия
    void argCheck(Map f, std::vector<std::reference_wrapper<Terms> > _args);
    void argCheck(Map f, std::vector<Terms*> _args);
public:
    ParenSymbol(std::vector<std::reference_wrapper<Terms> > _args);
    ParenSymbol(const ParenSymbol& one);
    // Применяется глубокое копирование
    ParenSymbol(std::vector<Terms*> _args);
    virtual ~ParenSymbol();

    virtual void print(std::ostream& out = std::cout) const override;

    const Terms* arg(size_t oneTwoThree) const
    { return args[oneTwoThree-1]; }
    bool operator==(const ParenSymbol& other) const
    {
        for (size_t i = 0; i < args.size(); ++i)
            if (!args[i]->doCompare(other.args[i]))
                return false;
        return true;
    }
};

class Term : public Terms, public Symbol, public ParenSymbol
{
public:
    Term(Symbol f, std::vector<std::reference_wrapper<Terms> > _args)
            : Terms(f.getType()), Symbol(f),
              ParenSymbol(_args) { argCheck(f, _args); }
    Term(std::pair<Symbol,
                   std::vector<std::reference_wrapper<Terms> > >
         pair) : Term(pair.first, pair.second) {}
    Term(const Term& one) : Terms(one), Symbol(one), ParenSymbol(one) {}
    Term(Symbol f, std::vector<Terms*> _args)
            : Terms(f.getType()), Symbol(f),
              ParenSymbol(_args) {argCheck(f, _args); }
    virtual ~Term() {}

    using Terms::getType;
    virtual Term* clone() const override { return (new Term(*this)); }
    virtual void print(std::ostream& out = std::cout) const override;

    virtual bool doCompare(const Terms* other) const override
    {
        if (const Term* t = dynamic_cast<const Term*>(other))
            return ((this->Symbol::operator==)(*t) && (this->ParenSymbol::operator==)(*t));
        else
            return false;
    }
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
    //todo vars\var Отложил до реализации вывода специализацией и обобщением
    QuantedTerm(QType _type, Variable _var, Term _term) :
            type(_type), var(_var), term(_term.clone()), Terms(logical_mt)
    {
        if (_term.getType() != logical_mt)
            throw std::invalid_argument("Квантификация не логического терма.\n");
    }
    QuantedTerm(QType _type, Variable _var, QuantedTerm _term) :
            type(_type), var(_var), term(_term.clone()), Terms(logical_mt) {}
    QuantedTerm(QType _type, Variable _var, Terms* _term) :
            type(_type), var(_var), term(_term->clone()), Terms(logical_mt)
    {
        if (Term* t = dynamic_cast<Term*>(_term))
        {
            if (t->getType() != logical_mt)
                throw std::invalid_argument("Квантификация не логического терма.\n");
        }
        else if (QuantedTerm* qt = dynamic_cast<QuantedTerm*>(_term)) {}
        else throw std::invalid_argument("Квантификация неизвестного вида.\n");
    }
    virtual ~QuantedTerm() {}
    virtual QuantedTerm* clone() const override { return (new QuantedTerm(*this)); }
    virtual void print(std::ostream& out = std::cout) const override;

    virtual bool doCompare(const Terms* other) const override
    {
        if (const QuantedTerm* q = dynamic_cast<const QuantedTerm*>(other))
            return ((type == q->type) && (var.doCompare(&q->var)) && (term->doCompare(q->term)));
        else
            return false;
    }
};

#endif //TEST_BUILD_LOGIC_HPP
