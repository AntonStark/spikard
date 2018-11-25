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
    lecture->defType("\\mathbb{N}");
    lecture->defSym("dvc(\\cdot)", {"\\mathbb{N}"}, "Set");
    lecture->defSym("#\\cdot", {"Set"}, "\\mathbb{N}");
    lecture->defSym(R"(\cdot = \cdot)", {"\\mathbb{N}", "\\mathbb{N}"}, "Logical");
    
    lecture->defSym(R"(\{ \\_ | \cdot \})", {"any", "any"}, "Set"); // fixme вместо последнего "any" должен ->
    // быть тип P: elem(Set) -> Logical, где elem(Set) обозначает тип элементов множества из второго аргумента

    /**
     * Ну привет!
     * Нужно вспомнить что сделано для разбора на уровне лексера.
     * Там сначала лексемы, ещё скобки, слои - разобраться.
     * То есть прежде чем писать дальнейший разбор нужно вспомнить что уже готово.
     * Остерегайся желания улучшить!11
     * Потом твори! Быстрее к жизни!
     */
    lecture->addTerm(R"(\{ n \in \mathbb{N} | #dvs(n) = 2 \})");      // { n \in \mathbb{N} | dvs(n) = {1, n} }

    std::cout << "SUCCESS";

    return 0;
}