//
// Created by anton on 20.01.19.
//

#include "connectives.hpp"
#include "primary.hpp"
#include "string_name.hpp"

/**
 * // fixme оптимизировать регистрацию Connectives в NSI
 * Поскольку в PrintableConnective передаётся symForm, приходится
 * во-1 генерировать её
 * во-2 хранить для печати имя в чистом виде
 *  двойная работа
*/

UnaryOperation::UnaryOperation(const AbstractName* name, const MathType* operandType,
                               const MathType* resultType, bool prefix)
    : PrintableConnective(produceSymForm(name, prefix)), _name(name), _operandType(operandType),
      _resultType(resultType), _prefix(prefix) {}
std::string UnaryOperation::print(AbstractTerm::Vector args) const {
    if (!check(args))
        return "";
    else {
        if (_prefix)
            return (_name->toStr() + args.front()->print());
        else
            return (args.front()->print() + _name->toStr());
    }
}

const AbstractName* UnaryOperation::produceSymForm(const AbstractName* ownName, bool prefix) {
    auto name = ownName->toStr();
    if (prefix)
        return new TexName(name + "\\cdot");
    else
        return new TexName("\\cdot" + name);
}

std::vector<NameMatchInfo>
UnaryOperation::match(const Parser2::LexemeSequence& target, const std::pair<size_t, size_t>& bounds) const {
    size_t targetLen = bounds.second - bounds.first;
    auto nameSeq = dynamic_cast<const TexName*>(_name)->getSeq();
    if (not (targetLen > nameSeq.size()))
        return {};

    /**
     * если операция префиксная, значит операнд правее
     * проверяем совпадение начала _name с target[*bounds], иначе с конца
     */
    auto targetCompStart = (_prefix
        ? target.begin() + bounds.first
        : target.begin() + (bounds.second - nameSeq.size()));
    if (std::equal(nameSeq.begin(), nameSeq.end(), targetCompStart)) {
        NameMatchInfo result(_name);
        if (_prefix)
            result.add(bounds.first + nameSeq.size(), bounds.second, _operandType);
        else
            result.add(bounds.first, bounds.second - nameSeq.size(), _operandType);
        return {result};
    } else
        return {};
}


BinaryOperation::BinaryOperation(const AbstractName* name, const MathType* leftType, const MathType* rightType,
                                 const MathType* resultType, BinaryOperation::Notation notation)
    : PrintableConnective(produceSymForm(name, notation)), _name(name), _leftType(leftType), _rightType(rightType),
      _resultType(resultType), _notation(notation) {}
bool BinaryOperation::check(AbstractTerm::Vector args) const {
    return (args.size() == 2
            &&  _leftType->comp(args.at(0)->getType())
            && _rightType->comp(args.at(1)->getType()));
}
std::string BinaryOperation::print(AbstractTerm::Vector args) const {
    if (!check(args))
        return "";
    else {
        switch (_notation) {
            case Notation::PREFIX :
                return (_name->toStr() + args.at(0)->print() + args.at(1)->print());
            case Notation::INFIX :
                return (args.at(0)->print() + _name->toStr() + args.at(1)->print());
            case Notation::POSTFIX :
                return (args.at(0)->print() + args.at(1)->print() + _name->toStr());
        }
    }
}

const AbstractName* BinaryOperation::produceSymForm(const AbstractName* ownName, BinaryOperation::Notation notation) {
    switch (notation) {
        case Notation::PREFIX :
            return new TexName(ownName->toStr() + R"(\cdot\cdot)");
        case Notation::INFIX :
            return new TexName("\\cdot" + ownName->toStr() + "\\cdot");
        case Notation::POSTFIX :
            return new TexName(R"(\cdot\cdot)" + ownName->toStr());
    }
}

std::vector<NameMatchInfo>
BinaryOperation::match(const Parser2::LexemeSequence& target, const std::pair<size_t, size_t>& bounds) const {
    auto nameSeq = dynamic_cast<const TexName*>(_name)->getSeq();
    size_t targetLen = bounds.second - bounds.first;
    size_t argsPartLen = targetLen - nameSeq.size();
    if (not (targetLen > nameSeq.size()))
        return {};

    /**
     * в случае не инфиксной бинарной операции не очень понятно как эффективно
     * разделить остальную строку на два аргумента
     *
     * тупое решение: генерировать вектор совпадений со всеми возможными разбиениями. среди
     * них будет верное и дерево рано или поздно придет к успеху. причем большинство
     * разбиений не будут иметь смысла, так что тупикове ветви будут коротки
     */
    switch (_notation) {
        case Notation::PREFIX : {
            auto targetCompStart = target.begin() + bounds.first;
            if (std::equal(nameSeq.begin(), nameSeq.end(), targetCompStart)) {
                // все возможные разбиения оставшейся подстроки
                std::vector<NameMatchInfo> results(argsPartLen - 1, NameMatchInfo(_name));
                for (size_t l = 1; l < argsPartLen; ++l) {
                    results[l-1].add(bounds.first + nameSeq.size(), bounds.first + nameSeq.size() + l, _leftType);
                    results[l-1].add(bounds.first + nameSeq.size() + l, bounds.second, _rightType);
                }
                return results;
            }
            else
                return {};
        }
        case Notation::INFIX : {
            std::vector<NameMatchInfo> results;
            // перебираем все возможные сдвиги имени собственно внутри target[bounds]
            for (size_t l = 1; l < argsPartLen; ++l) {
                if (std::equal(nameSeq.begin(), nameSeq.end(), target.begin() + bounds.first + l)) {
                    auto res = NameMatchInfo(_name);
                    res.add(bounds.first, bounds.first + l, _leftType);
                    res.add(bounds.first + l + nameSeq.size(), bounds.second, _rightType);
                    results.push_back(res);
                }
            }
            return results;
        }
        case Notation::POSTFIX : {
            auto targetCompStart = target.begin() + (bounds.second - nameSeq.size());
            if (std::equal(nameSeq.begin(), nameSeq.end(), targetCompStart)) {
                // все возможные разбиения оставшейся подстроки
                std::vector<NameMatchInfo> results(argsPartLen - 1, NameMatchInfo(_name));
                for (size_t l = 1; l < argsPartLen; ++l) {
                    results[l-1].add(bounds.first, bounds.first + l, _leftType);
                    results[l-1].add(bounds.first + l, bounds.second - nameSeq.size(), _rightType);
                }
                return results;
            }
            else
                return {};
        }
    }
}


bool SpecialConnective::check(AbstractTerm::Vector args) const {
    if (args.size() != getArity())
        return false;
    
    for (size_t i = 0; i < getArity(); ++i)
        if (not _argTypes[i]->comp(args[i]->getType()))
            return false;
    return true;
}

std::string SpecialConnective::print(AbstractTerm::Vector args) const {
    return std::string("<special connective print not implemented yet>"); // todo
}

std::vector<NameMatchInfo>
SpecialConnective::match(const Parser2::LexemeSequence& target, const std::pair<size_t, size_t>& bounds) const {
    auto findFirstFrom = [&target] (const Parser2::Lexeme& find, size_t from, size_t to) -> size_t {
        for (size_t i = from; i < to; ++i)
            if (target[i] == find)
                return i;
        return size_t(-1);
    };

    size_t start = bounds.first, end = bounds.second;
    NameMatchInfo nameMatch(_form);
    const Parser2::LexemeSequence& synFormSeq = dynamic_cast<const TexName*>(_form)->getSeq();
    size_t i = start, v = 0;
    size_t nArg = 0;
    while (v < synFormSeq.size()) {
        bool inputEnd = (i == end);
        if (inputEnd)
            return {};

        auto lexCat = texLexer.storage.which(synFormSeq[v]._id);
        bool isArgPlace = (lexCat == "argument_place");
        bool isVarPlace = (lexCat == "variable_place");
        if (not (isArgPlace || isVarPlace)) {
            if (synFormSeq[v] == target[i]) {
                ++i;
                ++v;
            } else
                return {};
        } else {
            bool variantEnding = (v == synFormSeq.size() - 1);
            // пропуск может стоять в конце (напр. \\cdot=\\cdot), тогда сразу успех
            if (variantEnding) {
                nameMatch.add(i, end, _argTypes[nArg++], isVarPlace);
                ++v;
                i = end;
            } else {
                const Parser2::Lexeme& nextLexeme = synFormSeq[v+1];
                size_t matchThat = findFirstFrom(nextLexeme, i, end);
                if (matchThat == size_t(-1)) {
                    return {};
                } else {
                    nameMatch.add(i, matchThat, _argTypes[nArg++], isVarPlace);
                    v+=2;   // впереди совпадение target и variant, проходим "argument_place" и само совпадение
                    i = matchThat + 1;
                }
            }
        }
    }
    if (i == end)
        return {nameMatch};
}

BinaryOperation* cartesian_product = new BinaryOperation(new StringName("\\times"), typeOfTypes, typeOfTypes, typeOfTypes);
BinaryOperation* map_symbol   = new BinaryOperation(new StringName("\\rightarrow"), typeOfTypes, typeOfTypes, typeOfTypes);
