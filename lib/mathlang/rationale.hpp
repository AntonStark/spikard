//
// Created by anton on 10.01.17.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <string>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
#include <sstream>
#include "../../json.hpp"

#include "mathtype.hpp"
#include "terms.hpp"
#include "structure.hpp"

using json = nlohmann::json;

PrimaryMT* getType(const NameSpaceIndex& index, const std::string& name);
Variable getVar (const NameSpaceIndex& index, const std::string& name);
std::set<Map> getSym(const NameSpaceIndex& index, const std::string& name);

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
    { r->process(this); return r->str(); }
};

class BranchNode;
class PrimaryNode : public NamedNode
/// Этот класс служит для группировки первичных понятий
{
protected:
    friend class BranchNode;
    friend class TermsBox;
    PrimaryNode(Node* parent, NameStoringStrategy* nss,
                NamedNodeType type, const std::string& title)
            : NamedNode(parent, nss, type, title) {}
    static PrimaryNode* fromJson(const json& j, BranchNode* parent = nullptr);
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
                 const std::vector<std::string>& argT, const std::string& retT);
    void addTerm(const std::string& term);
    void doMP   (const std::string& pPremise, const std::string& pImpl);
    void doSpec (const std::string& pToSpec,  const std::string& termVar);
    void doApply(const std::string& pTerm,    const std::string& pTheorem);
    void doEqual(const std::string& pTerm,    const std::string& pEquality);
    void doGen  (const std::string& pToGen,   const std::string& pToVar);
};

class BranchNode : public NamedNode
/// Это класс для группировки групп
{
protected:
    BranchNode(BranchNode* parent, NameStoringStrategy* nss,
               NamedNodeType type, const std::string& title)
            : NamedNode(parent, nss, type, title) {}
    static BranchNode* fromJson(const json& j, BranchNode* parent);
public:
    BranchNode(const std::string& title)
            : BranchNode(nullptr, new Hidden(), NamedNodeType::COURSE, title) {}
    static BranchNode* fromJson(const json& j) { return fromJson(j, nullptr); }
    ~BranchNode() override = default;

    void startCourse (const std::string& title = "")
    { new BranchNode(this, new Hidden(this), NamedNodeType::COURSE, title); }
    void startSection(const std::string& title = "")
    { new BranchNode(this, new Appending(this), NamedNodeType::SECTION, title); }
    void startLecture(const std::string& title = "")
    { new PrimaryNode(this, new Appending(this), NamedNodeType::LECTURE, title); }

    PrimaryNode* getSub(size_t number)
    { return static_cast<PrimaryNode*>(getByNumber(number)); }
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

class DefType : public AbstrDef, public PrimaryMT
{
private:
    friend class PrimaryNode;
    DefType(PrimaryNode* naming, const std::string& typeName)
            : AbstrDef(naming, NameTy::MT, typeName), PrimaryMT(typeName) {}
    static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);
public:
    ~DefType() override = default;
    DefType(const DefType&) = delete;
    DefType& operator=(const DefType&) = delete;

    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
};

class DefVar : public AbstrDef, public Variable
{
private:
    friend class PrimaryNode;
    DefVar(PrimaryNode* naming, const std::string& varName, const MathType* mathType)
            : AbstrDef(naming, NameTy::VAR, varName), Variable(varName, mathType) {}
    static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);
public:
    ~DefVar() override = default;
    DefVar(const DefVar&) = delete;
    DefVar& operator=(const DefVar&) = delete;

    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
};

class DefSym : public AbstrDef, public Map
{
private:
    friend class PrimaryNode;
    DefSym(PrimaryNode* naming, const std::string& symName,
           const std::vector<const MathType*>& argT, const MathType* retT)
            : AbstrDef(naming, NameTy::SYM, symName), Map(symName, argT, retT) {}
    static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);
public:
    ~DefSym() override = default;
    DefSym(const DefSym&) = delete;
    DefSym& operator=(const DefSym&) = delete;

    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
};

class Statement
{
public:
    virtual const Terms* get() const = 0;
};

class TermsBox : public PrimaryNode, public Statement
/// Этот класс представляет аксиомы. Наследование от PrimaryNode из-за
/// необходиомости хранить имена при кванторах
{
private:
    const Terms* data;

    friend class PrimaryNode;
    friend class Lexer;
    TermsBox(PrimaryNode* parent, std::string source);
    static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);
public:
    ~TermsBox() override = default;
    TermsBox(const TermsBox&) = delete;
    TermsBox& operator=(const TermsBox&) = delete;

    const Terms* get() const override { return data; }

    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
};
extern Terms* parse(PrimaryNode* where, std::string source);

class Inference : public Item, public Statement
// Этот класс представлет любые следствия.
{
public:
    class bad_inf;
    enum class InfTy {MP, SPEC, APPL, EQL, GEN};

    const std::vector<Path> premises;
    const InfTy type;
    const Terms* data;
protected:
    const Terms* getTerms(Path pathToTerm);
    const Terms* inference();
public:
    ~Inference() override = default;
    Inference(const Inference&) = delete;
    Inference& operator=(const Inference&) = delete;

    static Hierarchy* fromJson(const json& j, PrimaryNode* parent = nullptr);
    Inference(PrimaryNode* naming, Path pArg1, Path pArg2,  InfTy _type);

    const Terms* get() const override { return data; }
    std::string getTypeAsStr() const;
    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
};

Terms* modusPonens(const Terms* premise, const Terms* impl);
Terms* specialization(const Terms* general, const Terms* t);
Terms* application(const Terms* term, const Terms* theorem);
Terms* equalSubst(const Terms* term, const Terms* equality);
Term* generalization(const Terms* toGen, const Terms* x);

#endif //TEST_BUILD_SIGNATURE_HPP
