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
    DefAtom::create(lect1, "x", real);

    PrimaryNode* lect2 = PrimaryNode::create(main, "Лекция2", NameStoringStrategy::BasicNSSTypes::Appending);
    auto mult = DefConnective::create(lect1, "*", BinaryOperation::Notation::INFIX, real, real, real);
    /**
     * lect2 "сфотографировала" индекс lect1
     * mult создана в lect1 после этого
     * поэтому не видна в lect2 (вряд ли это полезное или предсказуемое поведение) fixme
     */

    cout << "Имена: " << endl;
    cout << lect2->index().names.debugPrint();

    cout << "Связки: " << endl;
    cout << lect2->index().connectives.debugPrint();

    cout << endl << "Uses of R" << endl;

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