//
// Created by anton on 02.02.19.
//

#include "../theorem_node.hpp"

int main() {
    BranchNode* main = new BranchNode("");
    PrimaryNode* lecture = PrimaryNode::create(main, "Лекция",
        NameStoringStrategy::BasicNSSTypes::Appending);
    TheoremNode* theorem = TheoremNode::create(lecture, "Нормальная подгруппа");

    auto groupType = DefType::create(lecture, "Group");
    auto logicalType = DefType::create(lecture, "Logical");
    auto subgroup = DefConnective::create(lecture, "\\subset", BinaryOperation::Notation::INFIX,
        groupType, groupType, logicalType);

    auto anyType = DefType::create(lecture, "Any");
    auto forall = DefConnective::create(lecture, R"(\forall \_ \cdot)", {anyType, logicalType}, logicalType);

    theorem->defineNames({{"N", groupType}, {"G", groupType}});


    int i = 0;
    return 0;
}