//
// Created by anton on 27.10.18.
//

#include "../proof/named_node.hpp"

int main() {
    BranchNode cource("Курс");
    cource.startLecture("Лекция");
    auto lecture = cource.getSub(1);

    lecture->defType("any");

    lecture->defType("Set");
    lecture->defType("Logical");
    lecture->defSym(R"(\cdot\in\cdot)", {"any", "Set"}, "Logical");

    lecture->defType("Map");
    lecture->defSym(R"(\cdot(\cdot))", {"Map", "any"}, "any");

    lecture->defType("\\mathbb{N}");
    lecture->defVar("dvc", "Map");  // todo по-хорошему тут должен быть уточняющий зависимый тип: не просто Map, а Map(\mathbb{N})

    lecture->defSym("#\\cdot", {"Set"}, "\\mathbb{N}");
    lecture->defSym(R"(\cdot = \cdot)", {"\\mathbb{N}", "\\mathbb{N}"}, "Logical");
    
    lecture->defSym(R"(\{ \_ | \cdot \})", {"any", "any"}, "Set"); // fixme вместо последнего "any" должен ->
    // быть тип P: elem(Set) -> Logical, где elem(Set) обозначает тип элементов множества из второго аргумента

//    lecture->addTerm(R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})");      // { n \in \mathbb{N} | dvs(n) = {1, n} }
    std::string input = R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})";
    Parser2::CurAnalysisData source = Parser2::texLexer.recognize(input);

    Parser2::Parser texParser(lecture);
    Parser2::NamesTree namesTree(source.filtered, texParser.namesDefined);
    namesTree.grow();

    for (const auto& elem : namesTree._treeStorage) {
        std::cout << elem._id << ":\t";
        std::cout << (elem.isBundle ? "[bundle]" : "[named]\t\t" + Parser2::texLexer.print(elem._name));
        std::cout << std::endl << "\t{";
        for (const auto& ch : namesTree._childrens[elem._id])
            std::cout << ch << ',';
        std::cout << "}";
        std::cout << std::endl << "\t" << Parser2::texLexer.print(namesTree.part(elem._bounds)) << std::endl;
    }

    return 0;
}