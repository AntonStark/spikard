//
// Created by anton on 10.01.17.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <string>
#include <set>
#include <stdexcept>

#include "logic.hpp"
#include "formulas.hpp"

template <typename V>
class UniqueNamedObjectFactory;

class Namespace
{
public:
    enum class NameTy {PRED, FUNC, CONS, VARS};
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

    const NameTy getFactoryType(const UniqueNamedObjectFactory<Predicate>* one)const { return NameTy::PRED; }
    const NameTy getFactoryType(const UniqueNamedObjectFactory<Function>* one) const { return NameTy::FUNC; }
    const NameTy getFactoryType(const UniqueNamedObjectFactory<Constant>* one) const { return NameTy::CONS; }
    const NameTy getFactoryType(const UniqueNamedObjectFactory<Variable>* one) const { return NameTy::VARS; }
};

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
    const Namespace::NameTy type;
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

class TermsFactory
{
private:
    Namespace& names;

    UniqueNamedObjectFactory<Constant> C;
    UniqueNamedObjectFactory<Variable> V;
    UniqueObjectFactory<std::pair<Function*,
                                  std::list<Terms*> >,
                        Term> T;
public:
    TermsFactory(Namespace& _names) : names(_names), C(names), V(names) {}
    ~TermsFactory() {}

    bool isCons(const std::string& name) const
    { return names.isThatType(name, Namespace::NameTy::CONS); }
    bool isVar(const std::string& name) const
    { return names.isThatType(name, Namespace::NameTy::VARS); }

    void addC(const std::string& name);
    void addV(const std::string& name);

    Constant* getC(const std::string& name) const;
    Variable* getV(const std::string& name) const;

    Term* makeTerm(Function* f, std::list<Terms*> args);
};

class FormulasFactory
{
private:
    UniqueObjectFactory<std::pair<Variable*, Modifier::MType>,
                        Modifier> M;
    UniqueObjectFactory<std::pair<Predicate*,
                                  std::list<Terms*> >,
                        Atom> A;
    UniqueObjectFactory<std::pair<Modifier*,
                                  std::pair<Formula*, Formula*> >,
                        ComposedF> F;
protected:
    Modifier* makeMod(Modifier::MType _type, Variable* _arg = nullptr);
public:
    FormulasFactory();
    ~FormulasFactory() {}

    Modifier* logNOT()  { return makeMod(Modifier::MType::NOT, nullptr); }
    Modifier* logAND()  { return makeMod(Modifier::MType::AND, nullptr); }
    Modifier* logOR()   { return makeMod(Modifier::MType::OR,  nullptr); }
    Modifier* logTHAN() { return makeMod(Modifier::MType::THAN,nullptr); }
    Modifier* forall(Variable* var) { return makeMod(Modifier::MType::FORALL, var); }
    Modifier* exists(Variable* var) { return makeMod(Modifier::MType::EXISTS, var); }

    Formula* makeFormula(Predicate* p, std::list<Terms*> args);
    Formula* makeFormula(Modifier* _mod, Formula* F1, Formula* F2 = nullptr);
};

class Signature
{
private:
    Namespace names;

    UniqueNamedObjectFactory<Predicate> R;
    UniqueNamedObjectFactory<Function> F;

    TermsFactory terms;
    FormulasFactory formulas;
protected:
    void addP(const std::string& name, unsigned arity);
    void addF(const std::string& name, unsigned arity);
    void addC(const std::string& name);
public:
    Signature() : names(), R(names), F(names), terms(names) {}
    Signature(std::list<std::pair<std::string, unsigned> > _R,
              std::list<std::pair<std::string, unsigned> > _F,
              std::list<std::string> _C);
    ~Signature() {}

    bool isPred(const std::string& name) const;
    bool isFunc(const std::string& name) const;
    bool isCons(const std::string& name) const;
    bool isVar(const std::string& name) const;

    void addV(const std::string& name);

    Predicate* getP(const std::string& name) const;
    Function* getF(const std::string& name) const;
    Constant* getC(const std::string& name) const;
    Variable* getV(const std::string& name) const;

    Term* makeTerm(Function* f, std::list<Terms*> args)
    { return terms.makeTerm(f, args); }
    Formula* makeFormula(Predicate* p, std::list<Terms*> args)
    { return formulas.makeFormula(p, args); }
    Formula* makeFormula(Modifier* _mod, Formula* F1, Formula* F2 = nullptr)
    { return formulas.makeFormula(_mod, F1, F2); }

    //TODO убрать в Namespace после возврата Interpr
    enum class nameT {predicate, function, constant, none};
    nameT checkName(const std::string& name) const;

    unsigned arity(const std::string& name) const;
    unsigned long maxLength(nameT type) const;
};

#endif //TEST_BUILD_SIGNATURE_HPP
