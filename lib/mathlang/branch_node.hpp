//
// Created by anton on 21.09.18.
//

#ifndef SPIKARD_MATHLANG_BRANCH_NODE_HPP
#define SPIKARD_MATHLANG_BRANCH_NODE_HPP

#include "named_node.hpp"
#include "primary_node.hpp"

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

#endif //SPIKARD_MATHLANG_BRANCH_NODE_HPP
