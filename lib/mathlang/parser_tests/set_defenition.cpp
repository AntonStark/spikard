//
// Created by anton on 27.10.18.
//

#include "../proof/named_node.hpp"
#include "../proof/definition.hpp"
#include "../parser/parser.hpp"

int main() {
    Parser2::texLexer = Parser2::Lexer::configureLatex();
    BranchNode cource("Курс");
    PrimaryNode* lecture = PrimaryNode::create(&cource, "Лекция",
                                               NameStoringStrategy::BasicNSSTypes::Appending);

    auto typeAny = Definition::create(lecture, "any");

    auto typeSet = Definition::create(lecture, "Set");
    auto typeLogical = Definition::create(lecture, "Logical");
    Definition::create(lecture, R"(\cdot\in\cdot)", {typeAny, typeSet}, typeLogical);

    auto typeMap = Definition::create(lecture, "Map");
    Definition::create(lecture, R"(\cdot(\cdot))", {typeMap, typeAny}, typeAny);

    auto typeN = Definition::create(lecture, "\\mathbb{N}");
    Definition::create(lecture, "2", typeN);
    Definition::create(lecture, "dvs", typeMap); // todo по-хорошему тут должен быть уточняющий зависимый тип: не просто Map, а Map(\mathbb{N}, \mathbb{N})

    Definition::create(lecture, "#\\cdot", {typeSet}, typeN);
    Definition::create(lecture, R"(\cdot = \cdot)", {typeN, typeN}, typeLogical);
    
    Definition::create(lecture, R"(\{ \_ | \cdot \})", {typeAny, typeAny}, typeSet); // fixme вместо последнего "any" должен ->
    // быть тип P: elem(Set) -> Logical, где elem(Set) обозначает тип элементов множества из второго аргумента

//    lecture->addTerm(R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})");      // { n \in \mathbb{N} | dvs(n) = {1, n} }
    std::string input = R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})";
    Parser2::CurAnalysisData source = Parser2::texLexer.recognize(input);

    Parser2::Parser texParser(lecture);
    Parser2::NamesTree namesTree(source.filtered, texParser.namesDefined);
    namesTree.grow();

    namesTree.debugPrint();

    return 0;
}