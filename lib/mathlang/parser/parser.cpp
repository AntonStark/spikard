//
// Created by anton on 05.09.18.
//

#include "parser.hpp"

namespace Parser2
{

void NameMatchInfo::add(size_t from, size_t to, bool isVarPlace) {
    _args.emplace_back(from, to, isVarPlace);
    if (isVarPlace)
        _varPlaces = true;
}

void matchWithGaps(const LexemeSequence& input, const std::pair<size_t, size_t>& bounds,
                   const AbstractName* variant, std::vector<NameMatchInfo>& forResults) {
    auto findFirstFrom = [&input] (const Lexeme& find, size_t from, size_t to) -> size_t {
        for (size_t i = from; i < to; ++i)
            if (input[i] == find)
                return i;
        return size_t(-1);
    };

    size_t start = bounds.first, end = bounds.second;
    NameMatchInfo nameMatch(variant);
    const LexemeSequence& variantLexems = dynamic_cast<const TexName*>(variant)->getSeq(); // todo вместо AbstractName лучше явно везде использовать TexName
    size_t i = start, v = 0;
    while (v < variantLexems.size()) {
        bool inputEnd = (i == end);
        if (inputEnd)
            return;

        auto lexCat = texLexer.storage.which(variantLexems[v]._id);
        bool isArgPlace = (lexCat == "argument_place");
        bool isVarPlace = (lexCat == "variable_place");
        if (not (isArgPlace || isVarPlace)) {
            if (variantLexems[v] == input[i]) {
                ++i;
                ++v;
            } else
                return;
        } else {
            bool variantEnding = (v == variantLexems.size() - 1);
            // пропуск может стоять в конце (напр. \\cdot=\\cdot), тогда сразу успех
            if (variantEnding) {
                nameMatch.add(i, end, isVarPlace);
                ++v;
                i = end;
            } else {
                const Lexeme& nextLexeme = variantLexems[v+1];
                size_t matchThat = findFirstFrom(nextLexeme, i, end);
                if (matchThat == size_t(-1)) {
                    return;
                } else {
                    nameMatch.add(i, matchThat, isVarPlace);
                    v+=2;   // впереди совпадение input и variant, проходим "argument_place" и само совпадение
                    i = matchThat + 1;
                }
            }
        }
    }
    if (i == end)
        forResults.push_back(nameMatch);
}

std::vector<NameMatchInfo> filter(const std::vector<const AbstractName*>& variants,
                                  const LexemeSequence& target, const std::pair<size_t, size_t>& bounds) {
    std::vector<NameMatchInfo> filtered;
    for (const auto& variant : variants)
        matchWithGaps(target, bounds, variant, filtered);
    return filtered;
}

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
        _getParent().registerName(name); // todo определять через дерево в каком узле регистрировать
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

/// Метод обработки отдельного узла, вызывается при появлении новых необработанных (под)строк
void NamesTreeElem::process() {
    auto namesDefined = index();
    const LexemeSequence& input = tree->_input;
    std::vector<NameMatchInfo> matches = filter(namesDefined, input, _bounds);

    if (_nameExpected) {
        if (matches.size() > 1)
             tree->setError("Ввод имени \"" + texLexer.print(tree->part(_bounds)) +
                             "\" допускает неоднозначный разбор. Не поддерживается.");
        else if (matches.size() == 1)
            becomeNamed(matches.front());
        else {
            LexemeSequence name = tree->part(_bounds);
            auto* texName = new TexName(name);  // fixme имя, объявленное таким образом, сдыхает as soon as
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


size_t NamesTree::_create(size_t parentId, const ElemBounds& bounds, bool name) {
    size_t thatElemId = _treeStorage.size();
    _treeStorage.emplace_back(this, thatElemId, bounds, name);
    _links.emplace_back(parentId);
    _links[parentId].childrens.push_back(thatElemId);
    return thatElemId;
}

bool NamesTree::hasParent(size_t id) const
{ return (id != 0); }

NamesTreeElem& NamesTree::elem(size_t id)
{ return _treeStorage[id]; }

LexemeSequence NamesTree::part(const ElemBounds& bouds) const
{ return LexemeSequence(_input.begin()+bouds.first, _input.begin()+bouds.second); }

void NamesTree::setError(const std::string& mess)
{ errorStatus = std::make_pair(true, mess); }

NamesTree::NamesTree(const LexemeSequence& input, const std::vector<const AbstractName*>& namedDefined) : _input(input) {
    _treeStorage.emplace_back(this, 0, std::make_pair(0, input.size()), false);
    _links.emplace_back(size_t(-1));

    _treeStorage.back()._ownNS = namedDefined;
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
        size_t elemCreatedId = _create(namedId, argInfo.bounds, argInfo.nameExpected | nameExpAcsedant);
        _forProcess.push(std::make_pair(argInfo.nameExpected, elemCreatedId)); // это вызовет рекурсивнй вызов process для соответсвующей подстроки
    }
}

/// Этот метод вызывается, когда хотим создать узел с именем (одно совпадение или для каждого случая)
void NamesTree::createNamed(size_t parentId, const NameMatchInfo& matchInfo) {
    // для узла ветвления на случаи (bounds установлен) вызывается becomeBundle
    // далее вызывается NamesTree::createCases, который просто объединяет несколько вызовов NamesTree::createNamed
    // короче, bounds можно получить из parent, который becomeBundle
    NamesTreeElem parent = elem(parentId);
    size_t elemId = _create(parentId, parent._bounds);
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
    } else
        detach(parentId); // todo проверка что parent есть, а то ошибка разбора
}

void NamesTree::debugPrint() {
    for (const auto& elem : _treeStorage) {
        std::cout << elem._id << ":\t";
        std::cout << (elem.isBundle ? "[bundle]" : "[named]\t\t" + (elem._name != nullptr ? elem._name->toStr() : "") + "\t");
        for (const auto& n : elem._ownNS)
            std::cout << n->toStr() << ", ";
        std::cout << std::endl << "\t{";
        for (const auto& ch : _links[elem._id].childrens)
            std::cout << ch << ',';
        std::cout << "}";
        std::cout << std::endl << "\t" << texLexer.print(part(elem._bounds)) << std::endl;
    }
}


std::vector<const AbstractName*> Parser::collectNames(const NameSpaceIndex& index)
{ return index.getNames(); }

Parser::Parser(Node* where)
    : _where(where),
      namesDefined(collectNames(where->index())) {}

AbstractTerm* Parser::parse(CurAnalysisData& source) {
    NamesTree namesTree(source.filtered, namesDefined);
    namesTree.grow();
}

AbstractTerm* parse(Node* where, std::string source) {
    Parser texParser(where);
    auto cad = texLexer.recognize(source);
    return texParser.parse(cad);
}

}
