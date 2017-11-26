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

enum class NamedNodeType {COURSE, SECTION, LECTURE, CLOSURE};
std::string toStr(NamedNodeType nnt);
class NamedNode : public Node, public Named
{
public:
    const NamedNodeType _type;
    ~NamedNode() override = default;
    NamedNode(Node* parent, NameStoringStrategy* nss,
              NamedNodeType type, const std::string& title)
            : Node(parent, nss), Named(title), _type(type) {}

    std::string print(Representation* r, bool incremental = true) const override
    { return r->process(this); }
};

class PrimaryNode : public NamedNode
/// Этот класс служит для группировки первичных понятий
{
protected:
    friend class BranchNode;
    friend class Axiom;
    PrimaryNode(Node* parent, NameStoringStrategy* nss,
                NamedNodeType type, const std::string& title)
            : NamedNode(parent, nss, type, title) {}
public:
    ~PrimaryNode() override = default;

    //  Таким образом есть два варианта организации размещения
    //  с соблюдением владения со стороны старшего в иерархии:
    //  1) DefType::create(closure, "Logical");
    //  2) closure->defType("Logical");
    //  Первый способ выглядит более громоздко, а второй
    //  перегружает интерфейс , но пусть так
    void defType(const std::string& typeName);
    void defVar (const std::string& varName, const std::string& typeName);
    void defSym (const std::string& symName,
                 const std::list<std::string>& argT, const std::string& retT);
    void addAxiom(const std::string& axiom);
    void doMP   (const std::string& pPremise, const std::string& pImpl);
    void doSpec (const std::string& pToSpec, const std::string& pToVar);
    void doGen  (const std::string& pToGen,  const std::string& pToVar);
};

class BranchNode : public NamedNode
/// Это класс для группировки групп
{
protected:
    BranchNode(BranchNode* parent, NameStoringStrategy* nss,
               NamedNodeType type, const std::string& title)
            : NamedNode(parent, nss, type, title) {}
public:
    BranchNode(const std::string& title)
            : BranchNode(nullptr, new Hidden(), NamedNodeType::COURSE, title) {}
    ~BranchNode() override = default;

    void startCourse (const std::string& title = "")
    { new BranchNode(this, new Hidden(this), NamedNodeType::COURSE, title); }
    void startSection(const std::string& title = "")
    { new BranchNode(this, new Appending(this), NamedNodeType::SECTION, title); }
    void startLecture(const std::string& title = "")
    { new PrimaryNode(this, new Appending(this), NamedNodeType::LECTURE, title); }

    Hierarchy* getSub(size_t number) { return getByNumber(number); }
};

class Item : public Hierarchy
/// Этот класс для первичных конструкций как определения и утверждения
{
protected:
    explicit Item(Node* parent) : Hierarchy(parent) {}

public:
    Hierarchy* getByPass(Path path) override
    { return (path.empty() ? this : nullptr); }
};

class AbstrDef : public Item
/// Это базовый класс определений, отвечает за регистрацию (тип, имя) в Namespace.
{
public:
    ~AbstrDef() override = default;
    AbstrDef(const AbstrDef&) = delete;
    AbstrDef& operator=(const AbstrDef&) = delete;

    AbstrDef(Node* parent, NameTy type, const std::string& name)
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

    json toJson() const override;
    json toMlObj() const override;*/
    std::string print(Representation* r, bool incremental) const override
    { return r->process(this); }
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

    json toJson() const override;
    json toMlObj() const override;*/
    std::string print(Representation* r, bool incremental) const override
    { return r->process(this); }
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

    json toJson() const override;
    json toMlObj() const override;*/
    std::string print(Representation* r, bool incremental) const override
    { return r->process(this); }
};

class Statement
{
public:
    virtual const Terms* get() const = 0;
};

class Axiom : public PrimaryNode, public Statement
/// Этот класс представляет аксиомы. Наследование от PrimaryNode из-за
/// необходиомости хранить имена при кванторах
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

    json toJson() const override;
    json toMlObj() const override;*/
    std::string print(Representation* r, bool incremental) const override
    { return r->process(this); }
};
extern Term* parse(Axiom* where, std::string source);

class AbstrInf : public Item, public Statement
// Этот класс представлет абстрактное следствие.
{
public:
    class bad_inf;
    enum class InfTy {MP, GEN, SPEC};
    const InfTy type;
    const std::vector<Path> premises;
protected:
    const Terms* getTerms(Path pathToTerm);
public:
    ~AbstrInf() override = default;
    AbstrInf(const AbstrInf&) = delete;
    AbstrInf& operator=(const AbstrInf&) = delete;

    AbstrInf(PrimaryNode* naming, InfTy _type, Path pArg1, Path pArg2)
            : Item(naming), premises({pArg1, pArg2}), type(_type) {}

    std::string getTypeAsStr() const;

    /*
    json toJson() const override;
    json toMlObj() const override;*/
    std::string print(Representation* r, bool incremental) const override
    { return r->process(this); }
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
