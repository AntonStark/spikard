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
    bool operator<(const MathType& other) const;

//    static MathType natural_mt, logical_mt, set;
};
extern MathType natural_mt;
extern MathType logical_mt;
extern MathType set_mt;

// Указание типов в отображнеии вовлекает семантику, но позволяет более полное описание сущности
// Важно, что с априорной информацией о типах упрощается парсер.
class Map
{
private:
    const MathType argT;
    const unsigned arity;
    const MathType retT;
public:
    Map(MathType _argT, unsigned _arity, MathType _retT)
            : arity(_arity), argT(_argT), retT(_retT) {}
    virtual ~Map() {}
    Map(const Map& one)
            : argT(one.argT), arity(one.arity), retT(one.retT) {}

    unsigned getArity() const { return arity; }
    bool operator== (const Map& one) const;
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
           MathType _argT, unsigned _arity, MathType _retT)
            : Label(_name), Map(_argT, _arity, _retT) {}

    virtual ~Symbol() {}
    Symbol(const Symbol& one)
            : Label(one), Map(one) {}
    bool operator== (const Symbol& one) const;
    typedef std::pair<std::string,
                      std::pair<std::pair<MathType, unsigned >,
                                MathType> > Sign;
    Symbol(const Sign& sign)
            : Symbol(sign.first,
                     sign.second.first.first, sign.second.first.second,
                     sign.second.second) {}
    bool operator<(const Symbol& other) const;
};

/*class Predicate : public Label, public Map
{
private:
    Predicate(const std::string& _name, unsigned _arity*//*, Signature* _sigma = nullptr*//*)
            : Label(_name), Map(_arity)*//*, sigma(_sigma)*//* {}
    template <typename V>
    friend class UniqueNamedObjectFactory;
public:
    virtual ~Predicate() {}
    Predicate(const Predicate& one)
            : Label(one), Map(one)*//*, sigma(one.sigma)*//* {}

    bool operator== (const Predicate& one) const;
};

class Function : public Label, public Map
{
private:
    Function(const std::string& _name, unsigned _arity)
            : Label(_name), Map(_arity) {}
    template <typename V>
    friend class UniqueNamedObjectFactory;
public:
    virtual ~Function() {}
    Function(const Function& one)
            : Label(one), Map(one) {}

    bool operator== (const Function& one) const;
};*/

class Terms : public virtual Printable
{
private:
    MathType type;
public:
    Terms(MathType _type) : type(_type) {}
    Terms(const Terms& one) : type(one.type) {}
    virtual ~Terms() {}
    virtual bool isVariable() const { return false; }
};

class Variable : public Terms, public Label
{
private:
    Variable(const std::string& _name, MathType _type)
            : Terms(_type), Label(_name) {}
    template <typename V>
    friend class UniqueNamedObjectFactory;
public:
    virtual ~Variable() {}
    Variable(const Variable& one) :
            Terms(one), Label(one) {}

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

class Term : public Terms, protected Symbol, public ParenSymbol
{
private:
    Term(Symbol* f, std::list<Terms*> _args)
            : Terms(f->getType()), Symbol(*f),
              ParenSymbol(_args) { argCheck(f, _args); }
    Term(std::pair<Symbol*, std::list<Terms*> > pair)
            : Term(pair.first, pair.second) {}
    template <typename K, typename V>
    friend class UniqueObjectFactory;
public:
    Term(const Term& one)
            : Terms(one), Symbol(one), ParenSymbol(one) {}
    virtual ~Term() {}

    virtual void print(std::ostream& out = std::cout) const override;
};

#endif //TEST_BUILD_LOGIC_HPP
