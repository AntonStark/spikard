//
// Created by anton on 10.01.17.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <string>
#include <set>
#include <stdexcept>

#include "logic.hpp"

class Namespace
{
private:
    class sym_doubling;
    class no_sym;

    std::set<std::string> predicates;
    std::set<std::string> functions;
    std::set<std::string> constants;
    std::set<std::string> variables;
public:
    bool isPred(const std::string& name) const;
    bool isFunc(const std::string& name) const;
    bool isCons(const std::string& name) const;
    bool isVar(const std::string& name) const;
    bool isSomeSym(const std::string& name) const;

    void checkPred(const std::string& name) const;
    void checkFunc(const std::string& name) const;
    void checkCons(const std::string& name) const;
    void checkVar(const std::string& name) const;
    void checkSym(const std::string& name) const;

    void addPred(const std::string& name);
    void addFunc(const std::string& name);
    void addCons(const std::string& name);
    void addVar(const std::string& name);
};

class TermsFactory
{
private:
    Namespace& names;

    std::map<std::string, Constant* > C;
    std::map<std::string, Variable* > V;
    std::map<std::pair<Function*,
                       std::list<Terms*> >,
             Term* > T;
public:
    TermsFactory(Namespace& _names) : names(_names) {}
    ~TermsFactory();

    bool isCons(const std::string& name) const
    { return names.isCons(name); }
    bool isVar(const std::string& name) const
    { return names.isVar(name); }

    Constant* getC(const std::string& name) const;
    Variable* getV(const std::string& name) const;

    void addC(const std::string& name);
    void addV(const std::string& name);

    Term* makeTerm(Function* f, std::list<Terms*> args);
};

class Signature
{
private:
    std::map<std::string, Predicate* > R;
    std::map<std::string, Function* > F;
    std::map<std::string, Constant* > C;

    Namespace names;
    TermsFactory termsStorage;
public:
    Signature() : names(), termsStorage(names) {}
    Signature(std::list<std::pair<std::string, unsigned> > _R,
              std::list<std::pair<std::string, unsigned> > _F,
              std::list<std::string> _C);
    ~Signature();

    bool isPred(const std::string& name) const
    { return (names.isPred(name)); }
    bool isFunc(const std::string& name) const
    { return (names.isFunc(name)); }
    bool isCons(const std::string& name) const
    { return (termsStorage.isCons(name)); }

    void addP(const std::string& name, unsigned arity);
    void addF(const std::string& name, unsigned arity);
    void addC(const std::string& name);

    Predicate* getP(const std::string& name) const;
    Function* getF(const std::string& name) const;
    Constant* getC(const std::string& name) const;

    //TODO убрать после возврата formulas
    enum class nameT {predicate, function, constant, none};
    nameT checkName(const std::string& name) const;

    unsigned arity(const std::string& name) const;
    unsigned long maxLength(nameT type) const;
};

#endif //TEST_BUILD_SIGNATURE_HPP
