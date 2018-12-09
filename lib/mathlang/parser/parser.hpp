//
// Created by anton on 05.09.18.
//

#ifndef SPIKARD_PARSER_HPP
#define SPIKARD_PARSER_HPP

#include <vector>

#include "lexer.hpp"
#include "../proof/named_node.hpp"

namespace Parser2
{

/// Отвечает за описание границ имени и его аргументных мест
struct NameGaps
{
    typedef std::pair<size_t, size_t> GapBounds;
    std::vector<GapBounds> _args;
    size_t _ownLength;
    size_t _fullLen;

    explicit NameGaps(size_t ownLength)
        : _ownLength(ownLength), _fullLen(ownLength) {}
    void add(size_t from, size_t to) {
        _args.emplace_back(from, to);
        _fullLen += (to - from);
    }
    size_t getFullLen()
    { return _fullLen; }
};

/**
 * Задача данного этапа - построить дерево частичного разбора. Листья такого дерева
 * могут быть как простыми (отдельные имена), так и составные (последовательности имён).
 * Далее останется определить порядок комбинации имён в составных листах и, наконец, собрать терм.
 *
 * Чтобы избежать манипулирования LexemeSequence, имя будет представлено парой
 * чисел [begin, end) - смещения над входной строкой. При сборке терма понадобится обход дерева сверху
 * вниз, значит узел должен хранить список своих потомков.
 *
 * Пока дерево не до конца выстроено будут возникать необработанне куски строки.
 * С точки зрения предыдущего узла они уже его потомки. Но их внутреннее устройство еще не выделено.
 * Таким образом есть четыре вида элементов дерева:
 * 0) "почка" - необработанная строка
 * 1) узел подчинения - имя, список потомков (напр. символ множества)
 * 2) узел сочинения - список потомков (например выражение с операциями)
 * 3) лист - имя
 *
 * храним все элементы в векторе treeStorage и понадобится вектор почек forProcess
 * элемент содержит:
 * 1) границы части строки за которую отвечает
 * 2) LexemeSequence (в случае узла сочинения пустую)
 * 3) список индексов treeStorage (в случае листа пустой)
 *
 * В начале создаём элемент с границами "вся строка" и помещаем в вектор почек
 * разбор идёт пока вектор почек не опустеет
 * при разборе строки, если встречается имя с пропусками, создаём почки для описания
 * пропусков и создаём узел для его описания с потомками - эти почки
 *
 * N.B. создавался один потомок - почка, а преверащается она в набор узлов, соотв. нужно ------ превращается в узел сочинения, всё ок
 *  обновлять список потомков - неправильно, что требуется модификация предка
 * N.B. Что с появлением новых имён внутри выражения? ------ они окажутся в почках для которых не удатся найти ни одного варианта
 * Как однако отличить новое имя от неудачи разбора???
 */

struct NamesTreeElem
{
    std::pair<size_t, size_t> _bounds;

    LexemeSequence _name;
    std::vector<size_t> _subElemsIndices;

    size_t _parent;
    bool isSymbolVars;
    NameSpaceIndex _ownNS;

    explicit NamesTreeElem(size_t from, size_t to)
        : _bounds(std::make_pair(from, to)) {}

    const NameSpaceIndex& index();
    void registerName(const LexemeSequence& name);
};

struct NamesTree
{
    const LexemeSequence& _input;
    std::vector<NamesTreeElem> _treeStorage;
    std::vector<size_t> _forProcess;

    NamesTree(const LexemeSequence& input) : _input(input) {
        _treeStorage.emplace_back(0, input.size());
        _forProcess.push_back(0);
    }
};

/**
 * В процессе разбора выражения могут появляться новые имена. Их область видимости будет ограничена
 * символом в котором были определены.
 *
 * Приписываем каждому узлу NSI. При создании дерева приписываем Parser::namesDefined.
 * Для остальных узлов NSI при создании пустой.
 *
 * И два метода:
 * index() для получения NSI в данном узле реализуется через рекурсивный вызов: родитель (узлу нужна ссылка
 *  на родителя) отдаёт свой NSI к которому добавлены (если ещё нет!) символы его родителя. Условие "если ещё нет" при
 *  обходе от потомков к родителю обеспечивает перегрузку имени.
 * register(name) для добавлению к родительскому NSI
 *
 * Причём! NSI создаются только (не считая корня) в узлах сочинения, имеющих аргументные места "ожидается имя",
 * Если NSI нет, метод register вызывает parent::register. Таким образом имя, определённое
 * в недрах терма-условия, будет видно в пределах символа, где оно ожидалось.
 *
 * По части работы с NSI получается структура аналогичная NamedNode.
 *
 * Чтобы разрешить проблему "тупик или новое имя" будем в конце считать кол-во имён, которые потребовалось
 * ввести в этом варианте и выбирать наименьший.
 */

struct PartialResolved
{
    typedef std::vector<LexemeSequence> result_type;

    size_t indent;
    result_type recognized;

    PartialResolved(size_t indent, result_type recognized)
        : indent(indent), recognized(std::move(recognized)) {}
    bool operator< (const PartialResolved& two) const
    { return (indent < two.indent); }
};

class Parser
{
public:
    const Node* _where;
    std::vector<LexemeSequence> namesDefined;

    Parser(Node* where);

    Terms* parse(CurAnalysisData& source);
};

Terms* parse(Node* where, std::string source);
}

#endif //SPIKARD_PARSER_HPP
