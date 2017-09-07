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

class Section;
class AbstrDef;
class NameSpaceIndex
{
public:
    enum class NameTy {SYM, VAR, MT};
private:
    std::map<std::string, NameTy> names;
    std::map<std::string, AbstrDef*> index;

    class name_doubling;
    class no_name;
public:
    void add(NameTy type, const std::string& name, AbstrDef* where);
    bool isThatType(const std::string& name, const NameTy& type) const;
    bool isSomeType(const std::string& name) const;

    std::set<std::string> getNames(NameTy type) const;
    MathType getT(const std::string& name) const;
    Variable getV(const std::string& name) const;
    Symbol   getS(const std::string& name) const;
};
typedef NameSpaceIndex::NameTy NameTy;

class Statement;
class Reasoning : virtual public Printable
{
public:
    const Reasoning* parent;
private:
    std::vector<Reasoning*> subs;
protected:
    Reasoning(Reasoning* _parent) : parent(_parent) {}
public:
    Reasoning() : parent(nullptr) {}
    virtual ~Reasoning();

    Reasoning* get(Path path);
    const Terms* getTerms(Path path) const;

    Terms* doMP  (const Terms* premise, const Terms* impl) const;
    Terms* doSpec(const Terms* general, const Terms* t) const;
//    Terms* doGen (const Terms* special, size_t numb, std::string name) const;

    bool deduceMP(Path rpPremise, Path rpImpl);
    bool deduceSpec(Path rpGeneral, Path rpT);
    bool deduceSpec(Path rpGeneral, Path subTermPath, Path rpT);

    virtual void print(std::ostream& out = std::cout) const override;
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
class Axiom;
class Section : public HierarchyItem
// Этот класс симулирует блок рассуждения и инкапсулирует работу с Namespace.
{
private:
    std::string title;
    NameSpaceIndex atTheEnd;    // Здесь хранится NSI, соответсвующее концу Section,
                                // потому что запись ведётся именно в конец.
                                // Вставки Def-ов влекут обновление.
    friend class Axiom;
    Section(Section*, const std::string& _title = "");
    Section(const Section&) = delete;
    Section& operator=(const Section&) = delete;

    friend class AbstrDef;
    void registerName(NameTy type, const std::string& name, AbstrDef* where);
public:
    Section(const std::string& _title = "");
    virtual ~Section() {}
    const NameSpaceIndex& index() const { return atTheEnd; }
    //  Таким образом есть два варианта организации размещения
    //  с соблюдением владения со стороны старшего в иерархии:
    //  1) DefType::create(closure, "Logical");
    //  2) closure->pushDefType("Logical");
    //  Первый способ выглядит более громоздко, а второй перегружает
    //  интерфейс Section посторонним функционалом, но пусть так
    void pushSection(const std::string& title = "");
    void pushDefType(const std::string& typeName);
    void pushDefVar (const std::string& varName, const std::string& typeName);
    void pushDefSym (const std::string& symName, const std::list<std::string>& argT, const std::string& retT);
    void pushAxiom  (const std::string& axiom);
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
    AbstrDef(Section* closure, NameTy type, const std::string& name)
            : HierarchyItem(closure) { closure->registerName(type, name, this); }
    virtual ~AbstrDef() {}
};

class DefType : public AbstrDef, public MathType
{
private:
    friend class Section;
    DefType(Section* closure, const std::string& typeName)
            : AbstrDef(closure, NameTy::MT, typeName), MathType(typeName) {}
    DefType(const DefType&) = delete;
    DefType& operator=(const DefType&) = delete;
public:
    virtual ~DefType() {}
};

class DefVar : public AbstrDef, public Variable
{
private:
    friend class Section;
    DefVar(Section* closure, const std::string& varName, MathType mathType)
            : AbstrDef(closure, NameTy::VAR, varName), Variable(varName, mathType) {}
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
           const std::list<MathType>& argT, MathType retT)
            : AbstrDef(closure, NameTy::SYM, symName), Symbol(symName, argT, retT) {}
    DefSym(const DefSym&) = delete;
    DefSym& operator=(const DefSym&) = delete;
public:
    virtual ~DefSym() {}
};

extern Term parse(Axiom* where, std::string source);
class Axiom : private Section, public Term
// Этот класс представляет аксиомы. Наследование от Section из-за
// необходиомости хранить переменные при кванторах
{
private:
    friend class Section;
    friend class Lexer;
    Axiom(Section* closure, std::string source);
    Axiom(const Axiom&) = delete;
    Axiom& operator=(const Axiom&) = delete;
public:
    virtual ~Axiom() {}
};

/*class Inference : public AStatement
// Этот класс представлет следствие.
{};*/
#endif //TEST_BUILD_SIGNATURE_HPP
