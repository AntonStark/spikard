//
// Created by anton on 21.09.18.
//

#ifndef SPIKARD_MATHLANG_PRIMARY_NODE_HPP
#define SPIKARD_MATHLANG_PRIMARY_NODE_HPP

#include "named_node.hpp"
#include "definition.hpp"
#include "statement.hpp"

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

#endif //SPIKARD_MATHLANG_PRIMARY_NODE_HPP
