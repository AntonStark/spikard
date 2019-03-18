//
// Created by anton on 05.09.18.
//

#ifndef SPIKARD_PARSER_HPP
#define SPIKARD_PARSER_HPP

#include <vector>
#include <queue>

#include "../basics/argument_place.hpp"
#include "../proof/named_node.hpp"
#include "../proof/definition.hpp"

#include "lexer.hpp"

namespace Parser2
{

/**
 * Задача данного этапа - построить дерево разбора имён. Точнее семейство всех возможных деревьев.
 *
 * Дерево храним в виде вектора элементов treeStorage и возможно понадобится второй вектор forProcess для необработанных элементов.
 *
 * Особенности элемента дерева:
 * - Пока дерево не до конца выстроено будут возникать необработанне куски строки.
 *  С точки зрения предыдущего узла они уже его потомки. Но их внутреннее устройство еще не выделено.
 * - Чтобы избежать манипулирования LexemeSequence, имя будет представлено парой чисел [begin, end) - смещения над входной строкой.
 * - При сборке терма понадобится обход дерева сверху вниз, значит узел должен хранить список своих потомков.
 * - В процессе разбора выражения могут появляться новые имена. Их область видимости будет ограничена
 * символом в котором были определены.
 * - Метод для получения NSI в данном узле реализуется через рекурсивный вызов: родитель (узлу нужна ссылка
 *  на родителя) отдаёт свой NSI к которому добавлены (если ещё нет!) символы его родителя. Условие "если ещё нет" при
 *  обходе от потомков к родителю обеспечивает перегрузку имени.
 * - Если узел не предполгает введения новых имён, регистрация происходит в родителе. Таким образом имя, определённое
 * в недрах терма-условия, будет видно в пределах символа, где оно ожидалось.
 * - В тех местах, где возможно сразу несколько вариантнов, дерево будет расслаиваться. То есть потомок может
 * превращаться в (виртуальный) узел расслоения.
 *
 * Устройство элемента:
 * 1) указатель на дерево (для получения родителя)
 * 2) id элемента в дереве
 * 3) границы части строки за которую отвечает
 * 4) отметка является ли виртуальным узлом расслоения
 * 5) само имя LexemeSequence
 * 6) отметка ожидается в данном узле ввод новых имён
 * 7) NSI
 *
 * Корень создавать виртуальным. После построения семейства деревьев для каждого будем считать кол-во
 * имён, которые потребовалось в нём ввести, и выбирать наименьший вариант.
 *
 * Ход роста дерева:
 * 1) Элемент имеет границы разбираемого участка строки и ссылку на родителя
 * 2) Получение NSI для данного узла
 * 3) Получаем список подходящих имён. Список может быть длины А) >1, Б) 1, В) 0
 *  A) Узел помечается как виртуальный, создаются потомки-случаи для каждого варианта
 *  Б) В качестве имени узла записывается найденный вариант. При необходимости выставляется отметка новых имён
 *  В) Данная последовательность целиком регистрируется как имя, завершение процесса роста.
 * А-4) Для каждого случая нужно создать потомков в соответствии с пропусками в имени, рекурсия к п. 1)
 * Б-4) Создаются узлы потомков для пропусков имени, рекурсия к п. 1)
 *
 * Элементу требуются функции:
 * - Конструктор от границ строки
 * - Доступ к родителю
 * - Получение списка имён (index())
 * - Регистрация имени (registerName(name))
 * - Метод приписывания имени узлу с созданием потомков по информации о пропусках в имени и выставлением отметки для NSI
 * - Метод становления узла виртуальным (bundle) с созданием случаев
 * - Для последнего нужен конструктор от информации о символе и пропусках (используя тот метод приписывания)
 */

/**
 * И последнее, что нужно использовать, это информация о Типах. Найдя подходящее имя, мы знаем
 * Типы его аргументов if any и можем сузить отсев имён на следующем шаге и не порождать заведомо ложные случаи.
 * Однако, имена, допускающие связывание (может быть полезно разделять на те, где связывание возможно и где обьязательно?), без
 * него имеют один Тип, а после другой. Например, Natural->Real становится просто Real.
 * Что если считать, что имя со связыванием имеет тот Тип, который будет после связывания? *
 * N.B. Функция! бывает запись f(x) а может быть (f+g)'(x)
 * при используемом методе работы с пропусками функция будет объявлена f(\cdot) и не будет признана подходящим
 * именем во втором случае.
 *  А если смотреть на скобки как на имя?
 *  И запятая как порождающий элемент декартового произведения, конкатенация списков!
 * Общий Тип Кортеж и Тип Функция, тогда скобки это конкструкция \cdot:Функция(\cdot:Кортеж)
 *
 * И, рассматривая скобки как одну из особых конструкций, можно перестать думать, что имя f поменяло
 * тип, став f(x). Раз и навсегда f:Map(Real, Real) а х:Real. Однако какой тогда Тип у \cdot:Функция(\cdot:Кортеж)?
 * Он зависит от того какой Тип у функции. Привет, зависимые Типы!
 * Пусть пока будет any, без зависимых Типов можно пока обойтись.
 */

struct NamesTree;
struct NamesTreeElem
{
    NamesTree* tree;
    size_t _id;
    ElemBounds _bounds;
    const MathType* _type;
    bool _nameExpected;

    bool isBundle;
    const AbstractName* _name;
    bool isSymbolVars;
    std::vector<const AbstractName*> _ownNS;

    NamesTreeElem(NamesTree* tree, size_t id, const ElemBounds& bounds, const MathType* resType, bool nameExected)
        : tree(tree), _id(id), _bounds(bounds), _type(resType), _nameExpected(nameExected) {}

    NamesTreeElem& _getParent() const;
    std::vector<const AbstractName*> index() const;
    void registerName(const AbstractName* name);

    void becomeNamed(const AbstractName* name);
    void becomeNamed(const NameMatchInfo& nameMatchInfo);
    void becomeBundle(const std::vector<NameMatchInfo>& matches);
    void process();
};

/**
 * Дерево содержит исходную последовательность, хранилище
 * элементов, а также следующую информацию:
 * - родителе каждого узла
 * - список потомков узла
 *
 * Родитель это просто отображение индекса элемента в хранилище в индекс родителя (vector<size_t>)
 * Список потомков аналогично - vector<Childrens>, где Childrens = vector<size_t>
 */

class Parser;
struct NamesTree
{
    struct Links
    {
        size_t parent;
        std::vector<size_t> children;

        explicit Links(size_t parentId)
        : parent(parentId) {}
        void _detach(size_t child) {
            for (auto it = children.begin(); it != children.end(); ++it) {
                if (*it == child) {
                    children.erase(it);
                    return;
                }
            }
        }

        bool hasOthers(size_t child) {
            for (const auto& ch : children) {
                if (ch != child)
                    return true;
            }
            return false;
        }
    };

    const LexemeSequence& _input;
    std::vector<NamesTreeElem> _treeStorage;
    std::vector<Links> _links;
    std::priority_queue<std::pair<bool, size_t> > _forProcess;

    Parser* _parser;
    std::pair<bool, std::string> errorStatus;

    size_t _create(size_t parentId, const ElemBounds& bounds, const MathType* type, bool name = false);
    bool hasParent(size_t id) const;
    NamesTreeElem& elem(size_t id);
    const NamesTreeElem& elem(size_t id) const;
    LexemeSequence part(const ElemBounds& bouds) const;
    void setError(const std::string& mess);

    NamesTree(const LexemeSequence& input, Parser* parser, const MathType* resType);
    void grow();

    void createArgs(size_t namedId, bool nameExpAcsedant, const NameMatchInfo& matchInfo);
    void createNamed(size_t parentId, const NameMatchInfo& matchInfo);
    void createCases(size_t parentId, const std::vector<NameMatchInfo>& matches);
    void detach(size_t id);

    void debugPrint();
};

class Parser
{
public:
    const Node* _where;
    std::vector<const AbstractName*> namesDefined;

//    std::vector<const AbstractName*> collectNames(const NameSpaceIndex& index);

    Parser(Node* where);

    AbstractTerm* generateTerm(Item* container, const NamesTree& namesTree, size_t id = 0);
    AbstractTerm* parse(Item* container, CurAnalysisData& source, const MathType* resType);
};

/// если третий аргумент отсутствует, будет распознан логический тип
AbstractTerm* parse(Item* container, std::string expr, DefType* exprType = nullptr);
}

#endif //SPIKARD_PARSER_HPP
