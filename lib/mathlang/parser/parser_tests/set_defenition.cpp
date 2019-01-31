//
// Created by anton on 27.10.18.
//

#include "../../proof/named_node.hpp"
#include "../../proof/definition.hpp"
#include "../parser.hpp"

int main() {
    BranchNode* cource = new BranchNode("Курс");
    PrimaryNode* lecture = PrimaryNode::create(cource, "Лекция",
                                               NameStoringStrategy::BasicNSSTypes::Appending);

    auto typeAny = DefType::create(lecture, "any");

    auto typeSet = DefType::create(lecture, "Set");
    auto typeLogical = DefType::create(lecture, "Logical");
    DefConnective::create(lecture, "\\in", BinaryOperation::Notation::INFIX, typeAny, typeSet, typeLogical);

    auto typeN = DefType::create(lecture, "\\mathbb{N}");
    DefAtom::create(lecture, "2", typeN);
    DefFunct::create(lecture, "dvs", typeN, typeSet);

    DefConnective::create(lecture, "\\#", true, typeSet, typeN);
    DefConnective::create(lecture, "=", BinaryOperation::Notation::INFIX, typeN, typeN, typeLogical);

    DefConnective::create(lecture, R"(\{ \_ | \cdot \})", {typeAny, typeAny}, typeSet);
    // быть тип P: elem(Set) -> Logical, где elem(Set) обозначает тип элементов множества из второго аргумента

//    lecture->addTerm(R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})");      // { n \in \mathbb{N} | dvs(n) = {1, n} }
    std::string input = R"(\{ n \in \mathbb{N} | \#dvs(n) = 2 \})";
    Parser2::CurAnalysisData source = Parser2::texLexer.recognize(input);

    Parser2::Parser texParser(lecture);
    Parser2::NamesTree namesTree(source.filtered, texParser.namesDefined);
    /*for (const auto& n : texParser.namesDefined)
        std::cout << n->toStr() << std::endl;*/
    namesTree.grow();

    namesTree.debugPrint();

    delete cource;
    return 0;
}