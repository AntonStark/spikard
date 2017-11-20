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

#include "structure.hpp"

using json = nlohmann::json;

MathType getType(const NameSpaceIndex& index, const std::string& name);
Variable getVar (const NameSpaceIndex& index, const std::string& name);
Symbol   getSym (const NameSpaceIndex& index, const std::string& name);

class BranchNode : public virtual Node
/// Это класс для группировки групп
{
protected:
    explicit BranchNode(BranchNode* parent): Node(parent) {}
public:
    BranchNode() : Node() {}
    ~BranchNode() override = default;

    void startCourse(const std::string& title = "")
    { new Course(this, title); }
    void startSection(const std::string& title = "")
    { new Section(this, title); }
    void startLecture(const std::string& title = "")
    { new Lecture(this, title); }

    Hierarchy* getSub(size_t number)
    { return getByNumber(number); }
};
class PrimaryNode : public virtual Node
/// Этот класс служит для группировки первичных понятий
{
protected:
    explicit PrimaryNode(Node* parent): Node(parent) {}
public:
    ~PrimaryNode() override = default;

    //  Таким образом есть два варианта организации размещения
    //  с соблюдением владения со стороны старшего в иерархии:
    //  1) DefType::create(closure, "Logical");
    //  2) closure->defType("Logical");
    //  Первый способ выглядит более громоздко, а второй
    //  перегружает интерфейс , но пусть так
    void defType(const std::string& typeName)
    { new DefType(this, typeName); }
    void defVar (const std::string& varName, const std::string& typeName)
    { new DefVar(this, varName, getType(index(), typeName)); }
    void defSym (const std::string& symName,
                 const std::list<std::string>& argT, const std::string& retT);

    void addAxiom(const std::string& axiom)
    { new Axiom(this, axiom); }
    void doMP   (const std::string& pPremise, const std::string& pImpl)
    { new InfMP(this, mkPath(pPremise), mkPath(pImpl)); }
    void doSpec (const std::string& pToSpec, const std::string& pToVar)
    { new InfSpec(this, mkPath(pToSpec), mkPath(pToVar)); }
    void doGen  (const std::string& pToGen,  const std::string& pToVar)
    { new InfGen(this, mkPath(pToGen), mkPath(pToVar)); }
};


class Course : public BranchNode, public Inner, public Named
{
private:
    friend class BranchNode;
    explicit Course(BranchNode* parent, const std::string& title = "")
            : BranchNode(parent), Inner(), Named(title) {}
public:
    explicit Course(const std::string& title = "") : Course(nullptr, title) {}
    ~Course() override = default;
    Course(const Course&) = delete;
    Course& operator=(const Course&) = delete;

};

class Section : public BranchNode, public Appending, public Named
{
private:
    size_t _n;
    friend class BranchNode;
    explicit Section(BranchNode* parent, const std::string& title = "")
            : BranchNode(parent), Appending(parent), Named(title),
              _n(getParent()->getNumber()) {}
public:
    ~Section() override = default;
    Section(const Section&) = delete;
    Section& operator=(const Section&) = delete;
};

class Lecture : public PrimaryNode, public Appending, public Named
/// Этот класс симулирует блок рассуждения и инкапсулирует работу с Namespace.
{
private:
    friend class BranchNode;
    explicit Lecture(BranchNode* parent, const std::string& title = "")
            : PrimaryNode(parent), Appending(parent), Named(title) {}
public:
    ~Lecture() override = default;
    Lecture(const Lecture&) = delete;
    Lecture& operator=(const Lecture&) = delete;

    /*static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);
    static Hierarchy* fromJsonE(const json& j)
    { return fromJson(j.at("ItemData")); }

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;*/
};

class Item : public Hierarchy
/// Этот класс для первичных конструкций как определения и утверждения
{
protected:
    explicit Item(Node* parent) : Hierarchy(parent) {}
};

class AbstrDef : public Item
// Это базовый класс определений, отвечает за регистрацию (тип, имя) в Namespace.
{
public:
    ~AbstrDef() override = default;
    AbstrDef(const AbstrDef&) = delete;
    AbstrDef& operator=(const AbstrDef&) = delete;

    AbstrDef(PrimaryNode* parent, NameTy type, const std::string& name)
            : Item(parent) { parent->registerName(type, name, this); }
};

class DefType : public AbstrDef, public MathType
{
private:
    friend class PrimaryNode;
    DefType(PrimaryNode* naming, const std::string& typeName)
            : AbstrDef(naming, NameTy::MT, typeName), MathType(typeName) {}
public:
    ~DefType() override = default;
    DefType(const DefType&) = delete;
    DefType& operator=(const DefType&) = delete;

    /*static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;*/
};

class DefVar : public AbstrDef, public Variable
{
private:
    friend class PrimaryNode;
    DefVar(PrimaryNode* naming, const std::string& varName, MathType mathType)
            : AbstrDef(naming, NameTy::VAR, varName), Variable(varName, mathType) {}
public:
    ~DefVar() override = default;
    DefVar(const DefVar&) = delete;
    DefVar& operator=(const DefVar&) = delete;

    /*static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;*/
};

class DefSym : public AbstrDef, public Symbol
{
private:
    friend class PrimaryNode;
    DefSym(PrimaryNode* naming, const std::string& symName,
           const std::list<MathType>& argT, MathType retT)
            : AbstrDef(naming, NameTy::SYM, symName), Symbol(symName, argT, retT) {}
public:
    ~DefSym() override = default;
    DefSym(const DefSym&) = delete;
    DefSym& operator=(const DefSym&) = delete;

    /*static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;*/
};

class Closure : public PrimaryNode, public Inner
{
private:
    friend class PrimaryNode;
    friend class Axiom;
    explicit Closure(PrimaryNode* parent, const std::string& title = "")
    : PrimaryNode(parent), Inner(parent), Named(title) {}
public:
    ~Closure() override = default;
    Closure(const Closure&) = delete;
    Closure& operator=(const Closure&) = delete;

    /*static Hierarchy* fromJson(const json& j, Closure* parent = nullptr);
    static Hierarchy* fromJsonE(const json& j)
    { return fromJson(j.at("ItemData")); }

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;*/
};

class Statement
{
public:
    virtual const Terms* get() const = 0;
};

extern Term* parse(Axiom* where, std::string source);
class Axiom : public Closure, public Statement
// Этот класс представляет аксиомы. Наследование от Closure из-за
// необходиомости хранить переменные при кванторах
{
private:
    const Terms* data;

    friend class PrimaryNode;
    friend class Lexer;
    Axiom(PrimaryNode* parent, std::string source);
public:
    ~Axiom() override = default;
    Axiom(const Axiom&) = delete;
    Axiom& operator=(const Axiom&) = delete;

    const Terms* get() const override { return data; }
    /*static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;*/
};

class AbstrInf : public Item, public Statement
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

    AbstrInf(PrimaryNode* naming, InfTy _type, Path pArg1, Path pArg2)
            : Item(naming), premises({pArg1, pArg2}), type(_type) {}

    std::string getTypeAsStr() const;

    /*std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;*/
};

Terms* modusPonens(const Terms* premise, const Terms* impl);
class InfMP : public AbstrInf
{
private:
    Terms* data;
    friend class PrimaryNode;
    InfMP(PrimaryNode* naming, Path pArg1, Path pArg2);
public:
    ~InfMP() override = default;
    InfMP(const InfMP&) = delete;
    InfMP& operator=(const InfMP&) = delete;

    const Terms* get() const override { return data; }

//    static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);
};

Terms* specialization(const Terms* general, const Terms* t);
class InfSpec : public AbstrInf
{
private:
    Terms* data;
    friend class PrimaryNode;
    InfSpec(PrimaryNode* naming, Path pArg1, Path pArg2);
public:
    ~InfSpec() override = default;
    InfSpec(const InfSpec&) = delete;
    InfSpec& operator=(const InfSpec&) = delete;

    const Terms* get() const override { return data; }

//    static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);
};

Term*   generalization  (const Terms* toGen, const Terms* x);
class InfGen : public AbstrInf
{
private:
    Terms* data;
    friend class PrimaryNode;
    InfGen(PrimaryNode* naming, Path pArg1, Path pArg2);
public:
    ~InfGen() override = default;
    InfGen(const InfGen&) = delete;
    InfGen& operator=(const InfGen&) = delete;

    const Terms* get() const override { return data; }

//    static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);
};

#endif //TEST_BUILD_SIGNATURE_HPP
