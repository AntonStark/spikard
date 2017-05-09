//
// Created by anton on 10.01.17.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <string>
#include <set>
#include <stdexcept>

#include "logic.hpp"
//#include "formulas.hpp"

template <typename V>
class UniqueNamedObjectFactory;

class Namespace
{
public:
    enum class NameTy {SYM, VAR, MT};
private:
    class sym_doubling;
    class no_sym;

    std::map<NameTy,
             std::set<std::string> > names;
public:
    Namespace();

    bool isThatType(const std::string& name, const NameTy& type) const;
    bool isSomeSym(const std::string& name) const;

    void checkSym(const std::string& name, const NameTy& type) const;
    void checkSym(const std::string& name) const;

    void addSym(const std::string& name, const NameTy& type);
    void delSym(const std::string& name, const NameTy& type);

    void viewSetOfNames(std::set<std::string>& set, const NameTy& type) const
    {
        for (auto& w : names.at(type))
            set.insert(w);
    }
};
typedef Namespace::NameTy NameTy;

/// Универсальный класс для описания иерархической единицы рассуждения
/// Реализуется, что в разных разделах одни символы могут обозначать различные вещи
class Reasoning
{
private:
    std::list<Reasoning*> subs;
    Reasoning* parent;
    Namespace names;

    std::map<std::string, Symbol  > syms;
    std::map<std::string, Variable> vars;
    std::map<std::string, MathType> types;

    Reasoning(Reasoning* _parent) : parent(_parent) {}
public:
    Reasoning() : parent(nullptr) {}
    virtual ~Reasoning()
    {
        for (auto& r : subs)
            delete r;
    }

    Reasoning& getParent() const { return *parent; }
    Reasoning& startSub()
    {
        subs.push_back(new Reasoning(this));
        return *subs.back();
    }
    void addSub(Reasoning* sub)
    {
        subs.push_back(sub);
    }

    const Reasoning* isNameExist(const std::string& name, const NameTy& type) const
    {
        if (names.isThatType(name, type))
            return this;
        else if (parent != nullptr)
            return parent->isNameExist(name, type);
        else
            return nullptr;
    }

    void addSym(const std::string& name, Symbol sym)
    {
        names.addSym(name, NameTy::SYM);
        syms.insert({name, sym});
    }
    void addVar(const std::string& name, Variable var)
    {
        names.addSym(name, NameTy::VAR);
        vars.insert({name, var});
    }
    void addType(const std::string& name, MathType type)
    {
        names.addSym(name, NameTy::MT);
        types.insert({name, type});
    }

    Symbol   getS(const std::string& name) const
    {
        const Reasoning* reas = isNameExist(name, NameTy::SYM);
        if (reas != nullptr)
            return reas->syms.at(name);
        else
            throw std::invalid_argument("No sym.\n");
    }
    Variable getV(const std::string& name) const
    {
        const Reasoning* reas = isNameExist(name, NameTy::VAR);
        if (reas != nullptr)
            return reas->vars.at(name);
        else
            throw std::invalid_argument("No sym.\n");
    }
    MathType getT(const std::string& name) const
    {
        const Reasoning* reas = isNameExist(name, NameTy::MT);
        if (reas != nullptr)
            return reas->types.at(name);
        else
            throw std::invalid_argument("No sym.\n");
    }

    void viewSetOfNames(std::set<std::string>& set, const NameTy& type) const
    {
        names.viewSetOfNames(set, type);
        if (parent != nullptr)
            parent->viewSetOfNames(set, type);
    }

//    void addStatement(std::string source);
};

class Statement : public Reasoning
{
public:
    Terms* monom;
    Statement(Terms* _monom)
            : monom(_monom) {}
    virtual ~Statement()
    { delete monom; }
};

class Signature
{
private:
    Namespace names;
    std::set<Symbol> S;

    friend class TermsFactory;
public:
    Signature() : names() {}
    Signature(std::initializer_list<Symbol> _S);
    ~Signature() {}

    bool isSym(const Symbol& name) const;
//    unsigned arity(const std::string& name) const;

    const Namespace& viewNS() const
    { return names; }
};
extern Signature logical_sign;

#endif //TEST_BUILD_SIGNATURE_HPP
