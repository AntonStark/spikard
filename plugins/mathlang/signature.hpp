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
    class name_doubling;
    class no_name;

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
public:
    const Reasoning* parent;
private:
    std::vector<Reasoning*> subs;
    Namespace names;
    std::map<std::string, Symbol  > syms;
    std::map<std::string, Variable> vars;
    std::map<std::string, MathType> types;

protected:
    Reasoning(Reasoning* _parent) : parent(_parent) {}
public:
    Reasoning() : parent(nullptr) {}
    virtual ~Reasoning();

    Reasoning* get(Path path);
    const Terms* getTerms(Path path) const;

    Reasoning* startSub();
    Statement* addSub(Terms* monom);

    void popBack() { subs.pop_back(); }

    Terms* doMP  (const Terms* premise, const Terms* impl) const;
    Terms* doSpec(const Terms* general, const Terms* t) const;
//    Terms* doGen (const Terms* special, size_t numb, std::string name) const;

    bool deduceMP(Path rpPremise, Path rpImpl);
    bool deduceSpec(Path rpGeneral, Path rpT);
    bool deduceSpec(Path rpGeneral, Path subTermPath, Path rpT);

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
    void printNamespace(std::ostream& out = std::cout) const; //for_debug
};

class Statement : virtual public Printable, public Reasoning
{
public:
    typedef std::set<Path> Premises;
private:
    const Terms* monom;
    Premises premise;
    std::string comment;
public:
    Statement(Reasoning* _parent, const Terms* _monom,
              const Premises& _premise = {}, std::string _comment = "")
            : Reasoning(_parent), monom(_monom), premise(_premise), comment(_comment) {}
    virtual ~Statement() {}

    virtual void print(std::ostream& out = std::cout) const override;
    const Terms* get() const { return monom; }
    void set(const Terms* _monom) { monom = _monom; }
};

//TODO ФУНКЦИОНАЛ ДОЛЖЕН БЫТЬ НЕОБХОДИМЫМ И ДОСТАТОЧНЫМ
class Section;
class HierarchyItem // Этот класс обеспечивает древовидную структуру. Ни больше ни меньше.
{
private:
    Section* parent;
    std::list<HierarchyItem*> subs;
    void push(HierarchyItem* sub) { subs.push_back(sub); }
protected:
    HierarchyItem(Section* _parent);
    Section* getParent() const { return parent; }
public:
    HierarchyItem() : parent(nullptr) {}
    virtual ~HierarchyItem();
    HierarchyItem(const HierarchyItem&) = delete;
    HierarchyItem& operator=(const HierarchyItem&) = delete;
};

class AbstrDef;
class Section : public HierarchyItem
// Этот класс симулирует блок рассуждения и инкапсулирует работу с Namespace.
{
private:
    std::string title;
    Namespace atTheEnd; // Здесь хранится NS, соответсвующее концу Section,
                        // потому что запись ведётся именно в конец.
                        // Вставки Def-ов влекут обновление.
    std::map<std::string, AbstrDef*> index;

    Section(Section*, const std::string& _title = "");
    Section(const Section&) = delete;
    Section& operator=(const Section&) = delete;

    friend class AbstrDef;
    void registerName(NameTy type, const std::string& name, AbstrDef* where);
    MathType getType(const std::string& typeName);
public:
    Section(const std::string& _title = "");
    virtual ~Section() {}

    //  Таким образом есть два варианта организации размещения
    //  с соблюдением владения со стороны старшего в иерархии:
    //  1) DefType::create(closure, "Logical");
    //  2) closure->pushDefType("Logical");
    //  Первый способ выглядит более громоздко, а второй перегружает
    //  интерфейс Section посторонним функционалом, но пусть так
    void pushDefType(std::string typeName);
    void pushDefVar(std::string varName, std::string typeName);
    void pushDefSym(std::string symName, std::list<std::string> argT, std::string retT);
};

/*class LineFactory
{
    Section* owner;
public:
    LineFactory(Section* _owner) : owner(_owner) {}
    virtual ~LineFactory() {}

    virtual void makeDefType(std::string& typeName) = 0;
};*/

class AbstrDef : public HierarchyItem
// Это базовый класс определений, отвечает за регистрацию (тип, имя) в Namespace.
{
private:
    AbstrDef(const AbstrDef&) = delete;
    AbstrDef& operator=(const AbstrDef&) = delete;
public:
    AbstrDef(Section* closure, NameTy type, const std::string& name);
    virtual ~AbstrDef() {}
};

class DefType : public AbstrDef, public MathType
{
private:
    friend class Section;
    DefType(Section* closure, const std::string& typeName);
    DefType(const DefType&) = delete;
    DefType& operator=(const DefType&) = delete;
public:
    virtual ~DefType() {}
};

class DefVar : public AbstrDef, public Variable
{
private:
    friend class Section;
    DefVar(Section* closure, const std::string& varName, MathType mathType);
    DefVar(const DefVar&) = delete;
    DefVar& operator=(const DefVar&) = delete;
public:
    virtual ~DefVar() {}
};

class DefSym : public AbstrDef, public Symbol
{
private:
    friend class Section;
    DefSym(Section* closure, const std::string& symName,
           std::list<MathType> argT, MathType retT);
    DefSym(const DefSym&) = delete;
    DefSym& operator=(const DefSym&) = delete;
public:
    virtual ~DefSym() {}
};
#endif //TEST_BUILD_SIGNATURE_HPP
