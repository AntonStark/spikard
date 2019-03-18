//
// Created by anton on 27.10.18.
//

#include "../../basics/tex_name.hpp"

#include "../../proof/named_node.hpp"
#include "../../proof/definition.hpp"
#include "../../proof/statement.hpp"

#include "../parser.hpp"

int main() {
    BranchNode* course = new BranchNode("Курс");
    PrimaryNode* lecture = PrimaryNode::create(course, "Лекция",
                                               NameStoringStrategy::BasicNSSTypes::Appending);

    auto typeAny = DefType::create(lecture, "any");

    auto typeSet = DefType::create(lecture, "Set");
    DefAtom::create(lecture, "Nat", typeSet);
    auto typeLogical = DefType::create(lecture, "Logical");
    DefConnective::create(lecture, "\\in", BinaryOperation::Notation::INFIX, typeAny, typeSet, typeLogical);

    auto typeN = DefType::create(lecture, "\\mathbb{N}");
    DefConnective::create(lecture, R"(( \cdot ))", {typeN}, typeN); // todo вообще говоря, это семейство связок ([type]) -> [type]
    DefAtom::create(lecture, "2", typeN);
    DefFunct::create(lecture, "dvs", typeN, typeSet);

    DefConnective::create(lecture, "\\#", true, typeSet, typeN);
    DefConnective::create(lecture, "=", BinaryOperation::Notation::INFIX, typeN, typeN, typeLogical);

    DefConnective::create(lecture, R"(\{ \_ | \cdot \})", {typeAny, typeLogical}, typeSet);
    // быть тип P: elem(Set) -> Logical, где elem(Set) обозначает тип элементов множества из второго аргумента

//    lecture->addTerm(R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})");      // { n \in \mathbb{N} | dvs(n) = {1, n} }
    std::string input = R"(\{ n \in Nat | \#dvs(n) = 2 \})";

//    Parser2::CurAnalysisData source = texLexer.recognize(input);
//    Parser2::Parser texParser(lecture);
    /*Parser2::NamesTree namesTree(source.filtered, &texParser, typeSet->use(nullptr));
    *//*for (const auto& n : texParser.namesDefined)
        std::cout << n->toStr() << std::endl;*//*
    namesTree.grow();

    namesTree.debugPrint();*/

//    auto res = texParser.parse(nullptr, source, typeSet->use(nullptr));
//    std::cout << res->print() << std::endl;
    auto inputed = TermsBox::create(lecture, input, typeSet);
    std::cout << "===RESULT===" << std::endl;
    std::cout << inputed->get()->print() << std::endl;
    std::cout << "===RESULT===" << std::endl;
    delete course;
    return 0;
}