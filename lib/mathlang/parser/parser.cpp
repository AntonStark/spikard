//
// Created by anton on 05.09.18.
//

#include "parser.hpp"

namespace Parser2
{

NamesTreeElem& NamesTreeElem::_getParent() const {
    size_t parentId = tree->_links[_id].parent;
    return tree->elem(parentId);
}

std::vector<const AbstractName*> NamesTreeElem::index() const {
    std::vector<const AbstractName*> own = _ownNS;
    if (tree->hasParent(_id)) {
        const auto& parentNS = _getParent().index();
        own.insert(own.end(), parentNS.begin(), parentNS.end());
    }
    return own;
}

void NamesTreeElem::registerName(const AbstractName* name) {
    if (isSymbolVars)
        _ownNS.push_back(name);
    else
        _getParent().registerName(name);
}

void NamesTreeElem::becomeNamed(const AbstractName* name) {
    _name = name;
    isBundle = false;
}

void NamesTreeElem::becomeNamed(const NameMatchInfo& nameMatchInfo) {
    becomeNamed(nameMatchInfo._name);
    isSymbolVars = nameMatchInfo.hasVarPlaces();
    tree->createArgs(_id, _nameExpected, nameMatchInfo);
}

void NamesTreeElem::becomeBundle(const std::vector<NameMatchInfo>& matches) {
    isBundle = true;
    isSymbolVars = false;
    tree->createCases(_id, matches);
}

std::vector<NameMatchInfo> select(Parser* parser, const LexemeSequence& input, std::pair<size_t, size_t> bounds,
    NameSpaceIndex::NamesSameType connectives, std::vector<const AbstractName*> local_names, NameSpaceIndex::NamesSameType names) {
    /**
     * обходим сначала связки. затем внутренние имена, затем внешние
     *
     * На векторе имён связок последовательно вызвать see()->match()
     * Успех даёт NMI/vector<NMI>(for binary)
     *
     * Если нет переходим к именам.
     * AbstractName is TexName is LexemeSequence так что это просто сопоставление двух векторов поэлементно.
     * Сначала для локальных имён, потом внешних
     * Если и эти два этапа прошли возвращаем пустой вектор - будет создано новое локальное имя.
     */
     NameSpaceIndex conIndex = parser->_where->index().connectives;
     for (const auto& cN : connectives) {
         auto c = dynamic_cast<const MatchCheckingConnective*>(conIndex.get(cN)->see());
         auto res = c->match(input, bounds);
         if (not res.empty())
             return res;
     }
     for (const auto& lN : local_names) {
         const LexemeSequence& variant = dynamic_cast<const TexName*>(lN)->getSeq();
         if (bounds.second - bounds.first == variant.size()
         && std::equal(variant.begin(), variant.end(), input.begin() + bounds.first))
             return {NameMatchInfo(lN)};
     }
    for (const auto& lN : names) {
        const LexemeSequence& variant = dynamic_cast<const TexName*>(lN)->getSeq();
        if (bounds.second - bounds.first == variant.size()
            && std::equal(variant.begin(), variant.end(), input.begin() + bounds.first))
            return {NameMatchInfo(lN)};
    }
    return {};
}

/// Метод обработки отдельного узла, вызывается при появлении новых необработанных (под)строк
void NamesTreeElem::process() {
    auto inplaceDefined = index();
    auto namesThatType = tree->_parser->_where->getNames(_type);
    const LexemeSequence& input = tree->_input;
    std::vector<NameMatchInfo> matches = select(tree->_parser, input, _bounds,
        namesThatType.connectives, inplaceDefined, namesThatType.names);

    if (_nameExpected) {
        if (matches.size() > 1)
             tree->setError("Ввод имени \"" + texLexer.print(tree->part(_bounds)) +
                             "\" допускает неоднозначный разбор. Не поддерживается.");
        else if (matches.size() == 1)
            becomeNamed(matches.front());
        else {
            LexemeSequence name = tree->part(_bounds);
            auto* texName = new TexName(name);
            registerName(texName);
            becomeNamed(texName);
        }
    } else {
        if (matches.size() > 1)
            becomeBundle(matches);
        else if (matches.size() == 1)
            becomeNamed(matches.front());
        else
            tree->detach(_id);
    }
}


size_t NamesTree::_create(size_t parentId, const ElemBounds& bounds, const MathType* type, bool name) {
    size_t thatElemId = _treeStorage.size();
    _treeStorage.emplace_back(this, thatElemId, bounds, type, name);
    _links.emplace_back(parentId);
    _links[parentId].children.push_back(thatElemId);
    return thatElemId;
}

bool NamesTree::hasParent(size_t id) const
{ return (id != 0); }

NamesTreeElem& NamesTree::elem(size_t id)
{ return _treeStorage[id]; }

const NamesTreeElem& NamesTree::elem(size_t id) const
{ return _treeStorage.at(id); }

LexemeSequence NamesTree::part(const ElemBounds& bouds) const
{ return LexemeSequence(_input.begin()+bouds.first, _input.begin()+bouds.second); }

void NamesTree::setError(const std::string& mess)
{ errorStatus = std::make_pair(true, mess); }

NamesTree::NamesTree(const LexemeSequence& input, Parser* parser, const MathType* resType)
: _input(input), _parser(parser) {
    _treeStorage.emplace_back(this, 0, std::make_pair(0, input.size()), resType, false);
    _links.emplace_back(size_t(-1));

    _forProcess.push(std::make_pair(false, 0));
}

void NamesTree::grow() {
    while (not _forProcess.empty()) {
        auto toProcess = _forProcess.top().second;
        _forProcess.pop();
        elem(toProcess).process();
        if (errorStatus.first) {
            std::cerr << errorStatus.second;
            return;
        }
    }
}

/// Этот метод вызывается когда нужно создать потомков именного узла
void NamesTree::createArgs(size_t namedId, bool nameExpAcsedant, const NameMatchInfo& matchInfo) {
    for (const auto& argInfo : matchInfo._args) {
        size_t elemCreatedId = _create(namedId, argInfo.bounds, argInfo._type, argInfo.nameExpected | nameExpAcsedant);
        _forProcess.push(std::make_pair(argInfo.nameExpected, elemCreatedId)); // это вызовет рекурсивнй вызов process для соответсвующей подстроки
    }
}

/// Этот метод вызывается, когда хотим создать узел с именем (одно совпадение или для каждого случая)
void NamesTree::createNamed(size_t parentId, const NameMatchInfo& matchInfo) {
    // для узла ветвления на случаи (bounds установлен) вызывается becomeBundle
    // далее вызывается NamesTree::createCases, который просто объединяет несколько вызовов NamesTree::createNamed
    // короче, bounds можно получить из parent, который becomeBundle
    // и ожидаемый тип, стало быть, тоже такой как у parent
    NamesTreeElem parent = elem(parentId);
    size_t elemId = _create(parentId, parent._bounds, parent._type);
    elem(elemId).becomeNamed(matchInfo);
}

/// Этот метод создаёт потомков узла случая
void NamesTree::createCases(size_t parentId, const std::vector<NameMatchInfo>& matches) {
    for (const auto& m : matches)
        createNamed(parentId, m);
}

/**
 * @brief Отцепить узел от дерева (тупиковый путь разбора)
 * @param id элемент к удалению
 * Если родительский узел - узел случаев, и у него есть дочерние кроме данного, данный
 * вычеркивается из дочерних. Иначе вызывается detach(parent).
 * Отцепленные элементы не удаляются, но больше не видны.
 */
void NamesTree::detach(size_t id) {
    size_t parentId = _links[id].parent;
    if (elem(parentId).isBundle) {
        if (_links[parentId].hasOthers(id)) {
            _links[parentId]._detach(id);
        } else
            detach(parentId);
    } else {
        if (not hasParent(parentId))
            setError("Все варианты разбора безуспешны.");
        else
            detach(parentId);
    }
}

void NamesTree::debugPrint() {
    for (const auto& elem : _treeStorage) {
        std::cout << elem._id << ":\t";
        std::cout << (elem.isBundle ? "[bundle]" : "[named]\t\t" + (elem._name != nullptr ? elem._name->toStr() : "") + "\t");
        for (const auto& n : elem._ownNS)
            std::cout << n->toStr() << ", ";
        std::cout << std::endl << "\t{";
        for (const auto& ch : _links[elem._id].children)
            std::cout << ch << ',';
        std::cout << "}";
        std::cout << std::endl << "\t" << texLexer.print(part(elem._bounds)) << std::endl;
    }
}

Parser::Parser(Node* where)
    : _where(where) {}

AbstractTerm* Parser::generateTerm(Item* container, const Parser2::NamesTree& namesTree, size_t id) {
    /**
     * После того как (если) получили удачное дерево получаем (use(container)) термы по
     * их именам в узлах и собираем терм. Если в выражении использовались внутренние переменные
     * создаются Variable пока типа any. В узлах bundle, имеющих больше одного child, берётся первый.
     */
    if (namesTree.elem(id).isBundle)
        id = namesTree._links[id].children.front();

    const AbstractName* elemName = namesTree.elem(id)._name;
    auto indices = namesTree._parser->_where->index();
    if (namesTree._links[id].children.empty()) {
        Definition* defTerm = indices.names.get(elemName);
        if (defTerm != nullptr) {
            auto* term = dynamic_cast<PrimaryTerm*>(defTerm->use(container));
            return term;
        }
        else
            return new Variable(elemName, &any_mt);
    }
    else {
        const std::vector<size_t>& children = namesTree._links[id].children;
        AbstractTerm::Vector args;
        for (auto ch : children)
            args.push_back(generateTerm(container, namesTree, ch));

        Definition* defCon = indices.connectives.get(namesTree.elem(id)._name);
        auto* conn = dynamic_cast<AbstractConnective*>(defCon->use(container));
        return new ComplexTerm(conn, args);
    }
}

AbstractTerm* Parser::parse(CurAnalysisData& source, DefType* exprType, Item* container) {
    /**
     * container используется двояко:
     *   1) получение имён нужного типа от Node parent(),
     *   2) use при сборке из дерева в терм
     *
     * парсер получает подсказку exprType какой тип должен
     * получиться и запрашивает имена данного типа по необходимости
     *
     */
    NamesTree namesTree(source.filtered, this, exprType->use(container));
    namesTree.grow();
    if (namesTree.errorStatus.first)
        throw std::runtime_error(namesTree.errorStatus.second);
    AbstractTerm* result = generateTerm(container, namesTree);
    return result;
}

AbstractTerm* parse(Item* container, std::string expr, DefType* exprType) {
    Parser texParser(container->getParent());
    auto cad = texLexer.recognize(expr);
    return texParser.parse(cad, exprType, container);
}

}
