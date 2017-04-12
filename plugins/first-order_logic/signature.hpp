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
    enum class NameTy {SYM, VAR};
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

    const NameTy getFactoryType(const UniqueNamedObjectFactory<Symbol>* one) const { return NameTy::SYM; }
    const NameTy getFactoryType(const UniqueNamedObjectFactory<Variable>* one) const { return NameTy::VAR; }

    friend class Lexer;
};
typedef Namespace::NameTy NameTy;

template <typename K, typename V>
class UniqueObjectStorage
{
protected:
    std::map<K, V*> storage;
public:
    virtual ~UniqueObjectStorage()
    {
        for (auto i : storage)
            delete i.second;
    }
};

template <typename K, typename V>
class UniqueObjectFactory : public UniqueObjectStorage<K, V>
{
protected:
    using UniqueObjectStorage<K, V>::storage;
public:
    virtual ~UniqueObjectFactory() {}

    bool check(const K& key) const
    {
        auto search = storage.find(key);
        return (search != storage.end());
    }
    V* make(const K& key)
    {
        auto search = storage.find(key);
        V* v;
        if (search != storage.end())
            v = search->second;
        else
            storage[key] = v = new V(key);
        return v;
    };
};

template <typename V>
class UniqueNamedObjectFactory : public UniqueObjectStorage<std::string, V>
{
private:
    Namespace& ns;
    const NameTy type;
protected:
    using UniqueObjectStorage<std::string, V>::storage;
public:
    UniqueNamedObjectFactory(Namespace& _ns)
            : ns(_ns), type(ns.getFactoryType(this)) {}
    virtual ~UniqueNamedObjectFactory()
    {
        for (auto i : storage)
            ns.delSym(i.first, type);
    }

    bool is(const std::string& key) const
    { return ns.isThatType(key, type); }

    V* get(const std::string& key) const
    {
        ns.checkSym(key, type);
        return storage.at(key);
    }
    void add(const std::string& name)
    {
        ns.addSym(name, type);
        storage[name] = new V(name);
    }
    void add(const std::string& name, unsigned arity)
    {
        ns.addSym(name, type);
        storage[name] = new V(name, arity);
    }
};

class Signature
{
private:
    Namespace names;

//    UniqueObjectFactory<Symbol::Sign, Symbol> S;
    std::set<Symbol> S;

    friend class TermsFactory;
public:
    Signature() : names() {}
    Signature(std::initializer_list<Symbol> _S);
    ~Signature() {}

    bool isSym(const Symbol& name) const;
//    Symbol* getS(const std::string& name) const;

//    unsigned arity(const std::string& name) const;

    const Namespace& viewNS() const
    { return names; }
};
extern Signature logical_sign;

/*class TermsFactory
{
private:
    Namespace& names;

    UniqueNamedObjectFactory<Variable> V;
    UniqueObjectFactory<std::pair<Symbol*,
                                  std::list<Terms*> >,
                        Term> T;
public:
    TermsFactory(Namespace& _names) : names(_names), V(names) {}
    TermsFactory(Signature& sigma) : names(sigma.names), V(names) {}
    ~TermsFactory() {}

    bool isVar(const std::string& name) const;
    Variable* getV(const std::string& name) const;
    void addV(const std::string& name);
    Variable* makeVar(const std::string& name);

    Term* makeTerm(Symbol* f, std::list<Terms*> args);
};*/

/*class FormulasFactory
{
private:
    UniqueObjectFactory<std::pair<Variable*, MType>,
                        Modifier> M;
    UniqueObjectFactory<std::pair<Predicate*,
                                  std::list<Terms*> >,
                        Atom> A;
    UniqueObjectFactory<std::pair<Modifier*,
                                  std::pair<FCard, FCard> >,
                        ComposedF> F;
//    std::set<Placeholder*> P;
protected:
    Modifier* makeMod(MType _type, Variable* _arg = nullptr);
public:
    FormulasFactory();
    ~FormulasFactory()
    {
//        for (auto p : P)
//            delete p;
    }

    Modifier* logNOT()  { return makeMod(MType::NOT); }
    Modifier* logAND()  { return makeMod(MType::AND); }
    Modifier* logOR()   { return makeMod(MType::OR ); }
    Modifier* logTHAN() { return makeMod(MType::THAN);}
    Modifier* forall(Variable* var) { return makeMod(MType::FORALL, var); }
    Modifier* exists(Variable* var) { return makeMod(MType::EXISTS, var); }

    FCard makeFormula(Predicate* p, std::list<Terms*> args);
    FCard makeFormula(Modifier* _mod, FCard F1, FCard F2 = nullptr);
    FCard makeFormula(Modifier::MType modT, FCard F1, FCard F2 = nullptr);
    FCard makeFormula(Modifier::MType modT, Variable* arg, Formula* F);
    FCard makeFormula(FCard base, std::stack<Formula::ArgTy> where, FCard forReplace);

    *//*Formula* makeFormula(Formula* one);
    Formula* makeFormula(ComposedF* cOne);

    Formula* makePlace();*//*
};*/

#endif //TEST_BUILD_SIGNATURE_HPP
