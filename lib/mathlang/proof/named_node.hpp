//
// Created by anton on 21.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMED_NODE_HPP
#define SPIKARD_MATHLANG_NAMED_NODE_HPP

#include "../basics/mathtype.hpp"
#include "../basics/terms.hpp"

#include "names_index.hpp"
#include "names_strategies.hpp"
#include "structure.hpp"

#include "definition.hpp"
#include "statement.hpp"

class NamedNode : public Node
{
public:
    enum class NNType {COURSE, SECTION, LECTURE, CLOSURE};
    const NNType _type;
    std::string _title;

    ~NamedNode() override = default;
    NamedNode(Node* parent, NameStoringStrategy* nss,
              NNType type, const std::string& title)
        : Node(parent, nss), _type(type), _title(title) {}
    NamedNode(Node* parent, NameStoringStrategy* nss,
              std::string type, const std::string& title)
        : NamedNode(parent, nss, nntFromStr(type), title) {}

    static std::string typeToStr(NNType nnt);
    static NNType nntFromStr(std::string str);

    std::string getName() const { return _title; }
    void setName(std::string name) { _title = std::move(name); }
    std::string print(Representation* r, bool incremental = true) const override
    { r->process(this); return r->str(); }
};

class BranchNode;
class PrimaryNode : public NamedNode
/// Этот класс служит для группировки первичных понятий
{
protected:
    PrimaryNode(Node* parent, NameStoringStrategy* nss,
                std::string type, const std::string& title)
        : NamedNode(parent, nss, type, title) {}
    PrimaryNode(Node* parent, NameStoringStrategy* nss,
                NamedNode::NNType type, const std::string& title)
        : NamedNode(parent, nss, type, title) {}

    friend class BranchNode;
    static PrimaryNode* fromJson(const json& j, Node* parent = nullptr);
public:
    ~PrimaryNode() override = default;

    //  Таким образом есть два варианта организации размещения
    //  с соблюдением владения со стороны старшего в иерархии:
    //  1) DefType::create(closure, "Logical");
    //  2) closure->defType("Logical");
    //  Первый способ выглядит более громоздко, а второй
    //  перегружает интерфейс , но пусть так
    void defType (const std::string& typeName);
    void defConst(const std::string& constName, const std::string& typeName);
    void defVar  (const std::string& varName, const std::string& typeName);
    void defSym  (const std::string& symForm,
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
               std::string type, const std::string& title)
        : NamedNode(parent, nss, type, title) {}
    BranchNode(BranchNode* parent, NameStoringStrategy* nss,
               NamedNode::NNType type, const std::string& title)
        : NamedNode(parent, nss, type, title) {}
    static BranchNode* fromJson(const json& j, BranchNode* parent);
public:
    BranchNode(const std::string& title)
        : BranchNode(nullptr, new Hidden(), NNType::COURSE, title) {}
    static BranchNode* fromJson(const json& j) { return fromJson(j, nullptr); }
    ~BranchNode() override = default;

    void startCourse (const std::string& title = "")
    { new BranchNode(this, new Hidden(this), NNType::COURSE, title); }
    void startSection(const std::string& title = "")
    { new BranchNode(this, new Appending(this), NNType::SECTION, title); }
    void startLecture(const std::string& title = "")
    { new PrimaryNode(this, new Appending(this), NNType::LECTURE, title); }

    PrimaryNode* getSub(size_t number)
    { return static_cast<PrimaryNode*>(getByNumber(number)); }
};

#endif //SPIKARD_MATHLANG_NAMED_NODE_HPP
