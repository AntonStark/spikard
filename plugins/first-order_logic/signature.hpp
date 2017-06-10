//
// Created by anton on 10.01.17.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <string>
#include <set>
#include <stdexcept>
#include <vector>

#include "logic.hpp"

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
    ~Namespace() {}

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
class Statement;
class Reasoning : virtual public Printable
{
private:
    std::vector<Reasoning*> subs;
    Reasoning* parent;
    Namespace names;

    std::map<std::string, Symbol  > syms;
    std::map<std::string, Variable> vars;
    std::map<std::string, MathType> types;

protected:
    Reasoning(Reasoning* _parent) : parent(_parent) {}
public:
    Reasoning() : parent(nullptr) {}
    virtual ~Reasoning();

    typedef std::list<size_t> RPath;

    Reasoning* getParent() const { return parent; }
    Reasoning* operator[] (size_t n) { return subs[n]; }
    Reasoning* get(RPath path);

    Reasoning& startSub();
    void addSub(Terms* monom);

    bool deduceMP(RPath premise, RPath impl);

    const Reasoning* isNameExist(const std::string& name,
                                 const NameTy& type) const;

    void addSym(Symbol sym);
    void addSym(std::list<Symbol> syms);
    void addSym(const std::string& name,
                std::list<MathType> argT, MathType retT);

    void addVar(Variable var);
    void addVar(const std::string& name, MathType type);

    void addType(MathType type);
    void addType(const std::string& name);

    Symbol   getS(const std::string& name) const;
    Variable getV(const std::string& name) const;
    MathType getT(const std::string& name) const;

    void viewSetOfNames(std::set<std::string>& set,
                        const NameTy& type) const;

    virtual void print(std::ostream& out = std::cout) const override;
};

class Statement : virtual public Printable, public Reasoning
{
    const Terms* monom;
    std::set<const Terms*> premise;
    std::string comment;
public:
    Statement(Reasoning* _parent, const Terms* _monom,
              const std::set<const Terms*> _premise = {}, std::string _comment = "")
            : Reasoning(_parent), monom(_monom), premise(_premise), comment(_comment) {}
    virtual ~Statement() {}

    virtual void print(std::ostream& out = std::cout) const override;
    const Terms* get() const
    { return monom; }
};

#endif //TEST_BUILD_SIGNATURE_HPP
