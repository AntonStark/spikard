//
// Created by anton on 08.02.19.
//

#include "../named_node.hpp"
#include "../definition.hpp"

using namespace std;

int main() {
    BranchNode* main = new BranchNode("");
    PrimaryNode* lect1 = PrimaryNode::create(main, "Лекция1", NameStoringStrategy::BasicNSSTypes::Appending);
    auto real = DefType::create(lect1, "R");
    DefConnective::create(lect1, "+", BinaryOperation::Notation::INFIX, real, real, real);
    DefConnective::create(lect1, "-", BinaryOperation::Notation::INFIX, real, real, real);

    PrimaryNode* lect2 = PrimaryNode::create(main, "Лекция2", NameStoringStrategy::BasicNSSTypes::Appending);
    auto mult = DefConnective::create(lect1, "*", BinaryOperation::Notation::INFIX, real, real, real);

    auto namesR = lect2->index().getNames(real->use(nullptr));
    /**
     * lect2 "сфотографировала" индекс lect1
     * mult создана в lect1 после этого
     * поэтому не видна в lect2 (вряд ли это полезное или предстказуемое поведение)
     */
    for (const auto& n : namesR)
        cout << n->toStr() << endl;

    cout << "Uses of R" << endl;

    auto usesR = real->uses();
    for (const auto& u: usesR) {
        if (u == nullptr)
            cout << "NULL" << endl;
        else {
            cout << dynamic_cast<NamedNode*>(u->getParent())->getName() << ": " << u->getNumber() << endl;
        }
    }

    return 0;
}