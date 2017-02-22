//
// Created by anton on 15.12.16.
//

#ifndef TEST_BUILD_FORMUALAS_HPP
#define TEST_BUILD_FORMUALAS_HPP

#include <stdexcept>
#include <map>
#include <list>
#include <set>

#include "logic.hpp"

class Modifier : public virtual Printable
{
public:
    enum class MType {NOT = 0, AND, OR, THAN, FORALL, EXISTS};
    static std::map<MType, const std::string> word;
private:
    class no_arg;
    class excess_arg;

    MType type;
    Variable* arg;

    bool isLogical(MType _type) const;
    bool isQuantor(MType _type) const;
    Modifier(MType _type, Variable* _arg = nullptr);
    Modifier(const std::pair<Variable*, MType>& pair)
            : Modifier(pair.second, pair.first) {}
    template <typename K, typename V>
    friend class UniqueObjectFactory;
public:
    Modifier(const Modifier& one) : type(one.type), arg(one.arg) {}
    virtual ~Modifier() {}

    void print(std::ostream& out = std::cout) const override;

    bool isLogical() const;
    bool isQuantor() const;
    bool isUnary() const;
    const MType getType() const;

//    Modifier* clone() const { return (new Modifier(*this)); }
};
typedef Modifier::MType MType;

class Formula : public virtual Printable
{
public:
    virtual bool isAtom() const { return false;};
    typedef std::set<const Placeholder*> PlacehSet;
    virtual PlacehSet getHolds() const = 0;
    virtual ~Formula() {};

    //TODO Если конструкторы ComposedF от ссылок всё-таки будут нужны, нужно
    //делать clone через фабричный метод make.
//    virtual Formula* clone() const = 0;
};

class Placeholder;
class ComposedF : public Formula
{
private:
    Formula *arg1, *arg2;
    Modifier* mod;
    PlacehSet holds;
    ComposedF(Modifier* _mod, Formula* F1, Formula* F2 = nullptr);
    ComposedF(const std::pair<Modifier*,
                        std::pair<Formula*, Formula*> >& tuple)
            : ComposedF(tuple.first, tuple.second.first, tuple.second.second) {}
    template <typename K, typename V>
    friend class UniqueObjectFactory;
public:
    ComposedF(const ComposedF& one)
            : arg1(one.arg1), mod(one.mod), arg2(one.arg2) {}
    /*ComposedF(const Modifier& _mod, const Formula& F);
    ComposedF(const Modifier& _mod, const Formula& F1, const Formula& F2);*/
    virtual ~ComposedF() {}

    void print(std::ostream& out = std::cout) const override;
//    ComposedF* clone() const override { return (new ComposedF(*this)); }

    const Modifier::MType getConType() const;
    Formula* getFArg() const;
    Formula* getSArg() const;
    PlacehSet getHolds() const override
    { return holds; }
};

class Atom : public Formula, protected Predicate, protected ParenSymbol
{
private:
    Atom(Predicate* p, std::list<Terms*> _args)
            : Predicate(*p), ParenSymbol(_args) { argCheck(p, _args); }
    Atom(const std::pair<Predicate*, std::list<Terms*> >& pair)
            : Atom(pair.first, pair.second) {}
    template <typename K, typename V>
    friend class UniqueObjectFactory;
public:
    Atom(const Atom& one)
            : Predicate(one), ParenSymbol(one) {}
    virtual ~Atom() {}

    void print(std::ostream& out = std::cout) const override;
    bool isAtom() const override { return true;}
//    Atom* clone() const override { return (new Atom(*this)); }
    PlacehSet getHolds() const override
    { return {}; }
};

class Placeholder : public Formula
{
private:
    unsigned ID;
    static unsigned inUse = 0;
public:
    Placeholder() : ID(inUse++) {}
    ~Placeholder() {};

    bool operator== (const Placeholder& other) const
    { return (ID == other.ID); }
    void print(std::ostream& out = std::cout) const override
    { out << '[' << ID << ']'; }
    PlacehSet getHolds() const override
    { return {this}; }
};

#endif //TEST_BUILD_FORMUALAS_HPP
