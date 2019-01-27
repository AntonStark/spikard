#include <utility>

//
// Created by anton on 21.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMED_NODE_HPP
#define SPIKARD_MATHLANG_NAMED_NODE_HPP

#include "../consepts/terms.hpp"

#include "../basics/mathtype.hpp"

#include "names_strategies.hpp"
#include "structure.hpp"

class NamedNode : public Node
{
public:
    std::string _title;

    ~NamedNode() override = default;
    NamedNode(Node* parent, NameStoringStrategy* nss, std::string title)
        : Node(parent, nss), _title(std::move(title)) {}

    std::string getName() const { return _title; }
    void setName(std::string name) { _title = std::move(name); }
    std::string print(Representation* r, bool incremental = true) const override
    { r->process(this); return r->str(); }
};

class BranchNode : public NamedNode
/// Это класс для группировки групп
{
protected:
    BranchNode(BranchNode* parent, NameStoringStrategy* nss, const std::string& title)
        : NamedNode(parent, nss, title) {}
public:
    BranchNode(const std::string& title)
        : BranchNode(nullptr, new Hidden(), title) {}
    ~BranchNode() override = default;
    static BranchNode* create(BranchNode* parent, const std::string& title,
                              NameStoringStrategy::BasicNSSTypes nssType)
    { return new BranchNode(parent, nssFromNSSType(nssType, parent), title); }

    Node* getSub(size_t number)
    { return static_cast<Node*>(getByNumber(number)); }
};

class PrimaryNode : public NamedNode
/// Этот класс служит для группировки первичных понятий
{
protected:
    PrimaryNode(Node* parent, NameStoringStrategy* nss,
                const std::string& title)
        : NamedNode(parent, nss, title) {}
public:
    ~PrimaryNode() override = default;
    //  Таким образом есть два варианта организации размещения
    //  с соблюдением владения со стороны старшего в иерархии:
    //  1) DefType::create(closure, "Logical");
    //  2) closure->defType("Logical");
    //  Первый способ выглядит более громоздко, а второй
    //  перегружает интерфейс , но пусть так
    //  Второй способ ещё создаёт зависимость классов узлов от классов
    //  элементов, что сильно осложняет расширение. Поэтому, возврат к 1)
    static PrimaryNode* create(BranchNode* parent,
                        const std::string& title, NameStoringStrategy::BasicNSSTypes nssType)
    { return new PrimaryNode(parent, nssFromNSSType(nssType, parent), title); }
/*    void addTerm(const std::string& term);
    void doMP   (const std::string& pPremise, const std::string& pImpl);
    void doSpec (const std::string& pToSpec,  const std::string& termVar);
    void doApply(const std::string& pTerm,    const std::string& pTheorem);
    void doEqual(const std::string& pTerm,    const std::string& pEquality);
    void doGen  (const std::string& pToGen,   const std::string& pToVar);*/
//    PrimaryMT* getType(const std::string& name);
};

#endif //SPIKARD_MATHLANG_NAMED_NODE_HPP
