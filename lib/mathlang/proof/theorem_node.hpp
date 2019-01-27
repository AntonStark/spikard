//
// Created by anton on 27.01.19.
//

#ifndef SPIKARD_THEOREM_NODE_HPP
#define SPIKARD_THEOREM_NODE_HPP

#include "named_node.hpp"
#include "definition.hpp"
#include "statement.hpp"

class TheoremNode : public BranchNode
{
private:
    PrimaryNode *names, *conditions, *conclusions; // shared_pointers
public:
    TheoremNode(BranchNode* parent, const std::string& title)
        : BranchNode(parent, new Hidden(parent), title) {
        names       = PrimaryNode::create(this, "Имена",   NameStoringStrategy::BasicNSSTypes::Appending);
        conditions  = PrimaryNode::create(this, "Условия", NameStoringStrategy::BasicNSSTypes::Appending);
        conclusions = PrimaryNode::create(this, "Выводы",  NameStoringStrategy::BasicNSSTypes::Appending);
    }

    typedef std::pair<std::string, DefType*> NameDef;
    void defineNames(const std::vector<NameDef>& namesInfo) {
        for (const auto& ni : namesInfo)
            DefAtom::create(names, ni.first, ni.second);
    }
    void appendCondition(const std::string& condition)
    { TermsBox::create(conditions, condition); }

    void assertConclusion(const std::string& conclusion)
    { TermsBox::create(conclusions, conclusion); }
};

#endif //SPIKARD_THEOREM_NODE_HPP
