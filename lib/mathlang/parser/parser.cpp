//
// Created by anton on 05.09.18.
//

#include "parser.hpp"

namespace Parser2
{

void NameMatchInfo::add(size_t from, size_t to, bool isVarPlace) {
    _args.emplace_back(from, to);
    if (isVarPlace)
        _varPlaces = true;
}

void matchWithGaps(const LexemeSequence& input, const std::pair<size_t, size_t>& bounds,
                   const LexemeSequence& variant, std::vector<NameMatchInfo>& forResults) {
    auto findFirstFrom = [&input] (const Lexeme& find, size_t from, size_t to) -> size_t {
        for (size_t i = from; i < to; ++i)
            if (input[i] == find)
                return i;
        return size_t(-1);
    };

    size_t start = bounds.first, end = bounds.second;
    NameMatchInfo nameMatch(variant);
    size_t i = start, v = 0;
    while (v < variant.size()) {
        bool inputEnd = (i == end);
        if (inputEnd)
            return;

        bool isArgPlace = (texLexer.storage.which(variant[v]._id) == "argument_place");
        if (not isArgPlace) {
            if (variant[v] == input[i]) {
                ++i;
                ++v;
            } else
                return;
        } else {
            bool isVarPlace = (texLexer.storage.get(variant[v]._id) == "\\_");
            bool noNextLexeme = (v == variant.size() - 1);
            // пропуск может стоять в конце (напр. \\cdot=\\cdot), тогда сразу успех
            if (noNextLexeme) {
                nameMatch.add(i, end, isVarPlace);
                ++v;
            } else {
                const Lexeme& nextLexeme = variant[v+1];
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
    forResults.push_back(nameMatch);
}

std::vector<NameMatchInfo> filter(const std::vector<LexemeSequence>& variants, const LexemeSequence& target, const std::pair<size_t, size_t>& bounds) {
    std::vector<NameMatchInfo> filtered;
    for (const auto& variant : variants)
        matchWithGaps(target, bounds, variant, filtered);
    return filtered;
}

NamesTreeElem& NamesTreeElem::_getParent() const
{ return _tree->get(_tree->_parent[_id]); }

std::vector<NamesType> NamesTreeElem::index() const {
    std::vector<NamesType> own = _ownNS;
    if (_tree->hasParent(_id)) {
        std::vector<NamesType> parent = _getParent().index();
        own.insert(own.end(), parent.begin(), parent.end());
    }
    return own;
}

void NamesTreeElem::registerName(const NamesType& name) {
    if (!isBundle)
        _ownNS.push_back(name);
    else
        _getParent().registerName(name);
}

void NamesTreeElem::becomeNamed(const NameMatchInfo& nameMatchInfo) {
    _name = nameMatchInfo._name;
    isBundle = false;
    isSymbolVars = nameMatchInfo.hasVarPlaces();
    _tree->createArgs(_id, nameMatchInfo);
}

void NamesTreeElem::becomeBundle(const std::vector<NameMatchInfo>& matches) {
    isBundle = true;
    isSymbolVars = false;
    _tree->createCases(_id, matches);
}

/// Метод обработки отдельного узла, вызывается при появлении новых необработанных (под)строк
void NamesTreeElem::process() {
    auto namesDefined = index();
    const LexemeSequence& input = _tree->_input;
    std::vector<NameMatchInfo> matches = filter(namesDefined, input, _bounds);
    if (matches.size() > 1)
        becomeBundle(matches);
    else if (matches.size() == 1)
        becomeNamed(matches.front());
    else
        registerName(_tree->part(_bounds));
}


size_t NamesTree::_create(size_t parentId, const NamesTreeElem::ElemBounds& bounds) {
    size_t elemCreatedId = _treeStorage.size();
    _treeStorage.emplace_back(this, elemCreatedId, bounds);
    _parent.push_back(parentId);
    _childrens.emplace_back();
    _childrens[parentId].push_back(elemCreatedId);
    return elemCreatedId;
}

bool NamesTree::hasParent(size_t id) const
{ return (_parent[id] != id); }

NamesTreeElem& NamesTree::get(size_t id)
{ return _treeStorage[id]; }

LexemeSequence NamesTree::part(const ElemBounds& bouds) const
{ return LexemeSequence(_input.begin()+bouds.first, _input.begin()+bouds.second); }

NamesTree::NamesTree(const LexemeSequence& input, const std::vector<LexemeSequence>& namedDefined) : _input(input) {
    size_t elemCreatedId = _create(0, std::make_pair(0, input.size()));
    _treeStorage.back()._ownNS = namedDefined;
    _forProcess.push(elemCreatedId);
}

void NamesTree::grow() {
    while (not _forProcess.empty()) {
        auto toProcess = _forProcess.top();
        _forProcess.pop();
        _treeStorage[toProcess].process();
    }
}

/// Этот метод вызывается когда нужно создать потомков именного узла
void NamesTree::createArgs(size_t parentId, const NameMatchInfo& matchInfo) {
    for (const auto& bounds : matchInfo._args) {
        size_t elemCreatedId = _create(parentId, bounds);

        _parent.push_back(parentId);
        _forProcess.push(elemCreatedId); // это вызовет рекурсивнй вызов prosecc для соответсвующей подстроки
    }
}

/// Этот метод вызывается, когда хотим создать узел с именем (одно совпадение или для каждого случая)
void NamesTree::createNamed(size_t parentId, const NameMatchInfo& matchInfo) {
    // для узла ветвления на случаи (bounds установлен) вызывается becomeBundle
    // далее вызывается NamesTree::createCases, который просто объединяет несколько вызовов NamesTree::createNamed
    // короче, bounds можно получить из parent, который becomeBundle
    auto bounds = _treeStorage[parentId]._bounds;
    size_t elemId = _create(parentId, bounds);
    _treeStorage[elemId].becomeNamed(matchInfo);
}

/// Этот метод создаёт потомков узла случая
void NamesTree::createCases(size_t parentId, const std::vector<NameMatchInfo>& matches) {
    for (const auto& m : matches)
        createNamed(parentId, m);
}


std::vector<LexemeSequence> Parser::collectNames(const NameSpaceIndex& index) {
    std::vector<LexemeSequence> storage;
    typedef NameSpaceIndex::NameTy NType;
    for (const auto& t : {NType::MT, NType::SYM, NType::VAR}) {
        const auto& namesThisType = index.getNames(t);
        for (const auto& name : namesThisType)
            storage.emplace_back(name);
    }
    return storage;
}

Parser::Parser(Node* where)
    : _where(where),
      namesDefined(collectNames(where->index())) {}

Terms* Parser::parse(CurAnalysisData& source) {
    NamesTree namesTree(source.filtered, namesDefined);
    namesTree.grow();

    /*for (const auto& elem : namesTree._treeStorage) {
        std::cout << elem._id << " : {";
        for (const auto& ch : namesTree._childrens[elem._id])
            std::cout << ch << ',';
        std::cout << "} " << std::endl;
        std::cout << "\t\t" << texLexer.print(namesTree.part(elem._bounds)) << std::endl;
        std::cout << (elem.isBundle ? "bundle" : texLexer.print(elem._name)) << std::endl << std::endl;
    }*/
}

Terms* parse(Node* where, std::string source) {
    Parser texParser(where);
    auto cad = texLexer.recognize(source);
    return texParser.parse(cad);
}

}
