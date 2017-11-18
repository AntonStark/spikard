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

class Section;
class HierarchyItem
// Этот класс обеспечивает древовидную структуру. Ни больше ни меньше.
{
private:
    Section* parent;
    std::list<HierarchyItem*> subs;
    mutable std::pair<bool, std::list<HierarchyItem*>::const_iterator> newInfo;
    void push(HierarchyItem* sub);
protected:
    HierarchyItem(Section* _parent);
    Section* getParent() const { return parent; }
    HierarchyItem* getByPass(Path path);
    void resetInfoFlag() { newInfo = {true, subs.begin()}; }
public:
    HierarchyItem() : parent(nullptr), newInfo({false, subs.end()}) {}
    virtual ~HierarchyItem();
    const Terms* getTerms(Path pathToTerm);
    size_t getNth() const;
    HierarchyItem(const HierarchyItem&) = delete;
    HierarchyItem& operator=(const HierarchyItem&) = delete;

    // У следующей функции такой странный дизайн, потому что хочется отдавать
    // MlObj для каждой конструкции новым сообщением (вызов write(InfoType, string)
    // А здесь до функционала плагина не добравться, в то же время нельзя пускать
    // плагин до внутренного устройства класса.
    void printMlObjIncr(std::list<std::string>& toOut) const;

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);

    virtual std::string toString() const;
    virtual json toJson() const;
    virtual json toMlObj() const = 0;
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

    friend class AbstrDef;
    void registerName(NameTy type, const std::string& name, AbstrDef* where);
public:
    ~Section() override = default;
    Section(const Section&) = delete;
    Section& operator=(const Section&) = delete;

    Section(const std::string& _title = "");
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

    std::string getTitle() const { return title; }
    void printB(std::ostream& out) const;

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
    static HierarchyItem* fromJsonE(const json& j)
    { return fromJson(j.at("ItemData")); }

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class AbstrDef : public HierarchyItem
// Это базовый класс определений, отвечает за регистрацию (тип, имя) в Namespace.
{
public:
    ~AbstrDef() override = default;
    AbstrDef(const AbstrDef&) = delete;
    AbstrDef& operator=(const AbstrDef&) = delete;

    AbstrDef(Section* closure, NameTy type, const std::string& name)
            : HierarchyItem(closure) { closure->registerName(type, name, this); }
};

class DefType : public AbstrDef, public MathType
{
private:
    friend class Section;
    DefType(Section* closure, const std::string& typeName)
            : AbstrDef(closure, NameTy::MT, typeName), MathType(typeName) {}
public:
    ~DefType() override = default;
    DefType(const DefType&) = delete;
    DefType& operator=(const DefType&) = delete;

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class DefVar : public AbstrDef, public Variable
{
private:
    friend class Section;
    DefVar(Section* closure, const std::string& varName, MathType mathType)
            : AbstrDef(closure, NameTy::VAR, varName), Variable(varName, mathType) {}
public:
    ~DefVar() override = default;
    DefVar(const DefVar&) = delete;
    DefVar& operator=(const DefVar&) = delete;

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class DefSym : public AbstrDef, public Symbol
{
private:
    friend class Section;
    DefSym(Section* closure, const std::string& symName,
           const std::list<MathType>& argT, MathType retT)
            : AbstrDef(closure, NameTy::SYM, symName), Symbol(symName, argT, retT) {}
public:
    ~DefSym() override = default;
    DefSym(const DefSym&) = delete;
    DefSym& operator=(const DefSym&) = delete;

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class Statement
{
public:
    virtual const Terms* get() const = 0;
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
public:
    ~Axiom() override = default;
    Axiom(const Axiom&) = delete;
    Axiom& operator=(const Axiom&) = delete;

    const Terms* get() const override { return data; }
    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
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
public:
    ~AbstrInf() override = default;
    AbstrInf(const AbstrInf&) = delete;
    AbstrInf& operator=(const AbstrInf&) = delete;

    AbstrInf(Section* closure, InfTy _type, Path pArg1, Path pArg2)
            : HierarchyItem(closure), premises({pArg1, pArg2}), type(_type) {}

    std::string getTypeAsStr() const;

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

Terms* modusPonens(const Terms* premise, const Terms* impl);
class InfMP : public AbstrInf
{
private:
    Terms* data;
    friend class Section;
    InfMP(Section* closure, Path pArg1, Path pArg2);
public:
    ~InfMP() override = default;
    InfMP(const InfMP&) = delete;
    InfMP& operator=(const InfMP&) = delete;

    const Terms* get() const override { return data; }

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

Terms* specialization(const Terms* general, const Terms* t);
class InfSpec : public AbstrInf
{
private:
    Terms* data;
    friend class Section;
    InfSpec(Section* closure, Path pArg1, Path pArg2);
public:
    ~InfSpec() override = default;
    InfSpec(const InfSpec&) = delete;
    InfSpec& operator=(const InfSpec&) = delete;

    const Terms* get() const override { return data; }

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

Term*   generalization  (const Terms* toGen, const Terms* x);
class InfGen : public AbstrInf
{
private:
    Terms* data;
    friend class Section;
    InfGen(Section* closure, Path pArg1, Path pArg2);
public:
    ~InfGen() override = default;
    InfGen(const InfGen&) = delete;
    InfGen& operator=(const InfGen&) = delete;

    const Terms* get() const override { return data; }

    static HierarchyItem* fromJson(const json& j, Section* parent = nullptr);
};

Path mkPath(std::string source);
#endif //TEST_BUILD_SIGNATURE_HPP
