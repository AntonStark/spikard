//
// Created by anton on 10.01.17.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <string>
#include <set>
#include <stdexcept>
#include <vector>
#include <sstream>
#include "../../json.hpp"

#include "logic.hpp"

class Section;
class AbstrDef;
class NameSpaceIndex
{
public:
    enum class NameTy {SYM, VAR, MT};
private:
    std::map<std::string, std::pair<NameTy, AbstrDef*> > data;

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

class HierarchyItem;
using json = nlohmann::json;
class Serializable
{
public:
    virtual json toJson() const = 0;
    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr)
    { return nullptr; }
};
class Section;
class HierarchyItem : public virtual Printable, public virtual Serializable
// Этот класс обеспечивает древовидную структуру. Ни больше ни меньше.
{
private:
    Section* parent;
    std::list<HierarchyItem*> subs;
    void push(HierarchyItem* sub) { subs.push_back(sub); }
protected:
    HierarchyItem(Section* _parent);
    Section* getParent() const { return parent; }
    HierarchyItem* get(Path path);
public:
    HierarchyItem() : parent(nullptr) {}
    virtual ~HierarchyItem();
    const Terms* getTerms(Path pathToTerm);
    HierarchyItem(const HierarchyItem&) = delete;
    HierarchyItem& operator=(const HierarchyItem&) = delete;

    virtual void print(std::ostream& out) const override;
    virtual json toJson() const override;
    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

class AbstrDef;
class Axiom;
class Section : public HierarchyItem, public virtual Printable
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
    //  2) closure->defType("Logical");
    //  Первый способ выглядит более громоздко, а второй перегружает
    //  интерфейс Section посторонним функционалом, но пусть так
    void startSection(const std::string& title = "");
    Section* getSub(const std::string& pToSub);
    void defType(const std::string& typeName);
    void defVar (const std::string& varName, const std::string& typeName);
    void defSym (const std::string& symName,
                 const std::list<std::string>& argT, const std::string& retT);
    void addAxiom(const std::string& axiom);
    void doMP   (const std::string& pPremise, const std::string& pImpl);
    void doSpec (const std::string& pToSpec, const std::string& pToVar);
    void doGen  (const std::string& pToGen,  const std::string& pToVar);

    virtual void print(std::ostream& out) const override;
    void printB(std::ostream& out) const;

    virtual json toJson() const override;
    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
    static HierarchyItem* fromJsonE(const json& j)
    { return fromJson(j.at("ItemData")); }
};

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
    virtual void print(std::ostream& out) const override;
    json toJson() const override;
    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
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
    virtual void print(std::ostream& out) const override;
    json toJson() const override;
    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
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
    virtual void print(std::ostream& out) const override;
    json toJson() const override;
    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

class Statement : public virtual Printable
{
public:
    virtual const Terms* get() const = 0;
    virtual void print(std::ostream& out) const override
    { out << *get(); }
};

extern Term* parse(Axiom* where, std::string source);
class Axiom : public Section, public Statement
// Этот класс представляет аксиомы. Наследование от Section из-за
// необходиомости хранить переменные при кванторах
{
private:
    const Terms* data;

    friend class Section;
    friend class Lexer;
    Axiom(Section* closure, std::string source);
    Axiom(const Axiom&) = delete;
    Axiom& operator=(const Axiom&) = delete;
public:
    virtual ~Axiom() {}
    virtual const Terms* get() const override { return data; }
    virtual void print(std::ostream& out) const override;

    json toJson() const override;
    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

class AbstrInf : public HierarchyItem, public Statement
// Этот класс представлет абстрактное следствие.
{
public:
    enum class InfTy {MP, GEN, SPEC};
    class bad_inf;
private:
    std::vector<Path> premises;
    InfTy type;
    
    AbstrInf(const AbstrInf&) = delete;
    AbstrInf& operator=(const AbstrInf&) = delete;
public:
    AbstrInf(Section* closure, InfTy _type, Path pArg1, Path pArg2)
            : HierarchyItem(closure), premises({pArg1, pArg2}), type(_type) {}
    virtual ~AbstrInf() {}
    virtual void print(std::ostream& out) const override;
    virtual json toJson() const override;
};

Terms* modusPonens(const Terms* premise, const Terms* impl);
class InfMP : public AbstrInf
{
private:
    Terms* data;
    friend class Section;
    InfMP(Section* closure, Path pArg1, Path pArg2);
    InfMP(const InfMP&) = delete;
    InfMP& operator=(const InfMP&) = delete;
public:
    virtual ~InfMP() {}
    virtual const Terms* get() const override { return data; }

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

Terms* specialization(const Terms* general, const Terms* t);
class InfSpec : public AbstrInf
{
private:
    Terms* data;
    friend class Section;
    InfSpec(Section* closure, Path pArg1, Path pArg2);
    InfSpec(const InfSpec&) = delete;
    InfSpec& operator=(const InfSpec&) = delete;
public:
    virtual ~InfSpec() {}
    virtual const Terms* get() const override { return data; }

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

Term*   generalization  (const Terms* toGen, const Terms* x);
class InfGen : public AbstrInf
{
private:
    Terms* data;
    friend class Section;
    InfGen(Section* closure, Path pArg1, Path pArg2);
    InfGen(const InfGen&) = delete;
    InfGen& operator=(const InfGen&) = delete;
public:
    virtual ~InfGen() {}
    virtual const Terms* get() const override { return data; }

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

Path mkPath(std::string source);
#endif //TEST_BUILD_SIGNATURE_HPP
