//
// Created by anton on 27.10.18.
//

#include "../proof/named_node.hpp"
#include "../proof/definition.hpp"
#include "../parser/parser.hpp"

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
    auto typeMap = DefType::create(lecture, "Map");
    Definition::create(lecture, "dvs", typeMap); // fixme Function todo по-хорошему тут должен быть уточняющий зависимый тип: не просто Map, а Map(\mathbb{N}, \mathbb{N})

    DefConnective::create(lecture, "#", true, typeSet, typeN);
    DefConnective::create(lecture, "=", BinaryOperation::Notation::INFIX, typeN, typeN, typeLogical);
    
    Definition::create(lecture, R"(\{ \_ | \cdot \})", {typeAny, typeAny}, typeSet); // fixme SpecialConnective fixme вместо последнего "any" должен ->
    // быть тип P: elem(Set) -> Logical, где elem(Set) обозначает тип элементов множества из второго аргумента

//    lecture->addTerm(R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})");      // { n \in \mathbb{N} | dvs(n) = {1, n} }
    std::string input = R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})";
    Parser2::CurAnalysisData source = Parser2::texLexer.recognize(input);

    Parser2::Parser texParser(lecture);
    Parser2::NamesTree namesTree(source.filtered, texParser.namesDefined);
    namesTree.grow();

    namesTree.debugPrint();

    delete cource;
    return 0;
}