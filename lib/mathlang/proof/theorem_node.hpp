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
    TheoremNode(PrimaryNode* parent, const std::string& title)
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

    static TheoremNode* create(PrimaryNode* parent, const std::string& title) {
        // todo регистрация в индексе теорем
        //      ключ - название, значение - сам узел
        //      узел нужен, чтобы иметь акутуальную информацию о кол-ве имён
        return new TheoremNode(parent, title);
    }
};

#endif //SPIKARD_THEOREM_NODE_HPP
