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
    size_t targetLen = bounds.second - bounds.first;
    auto nameSeq = dynamic_cast<const TexName*>(_name)->getSeq();
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
    NameMatchInfo result(_name);
    switch (_notation) {
        case Notation::PREFIX : {


            break;
        }
        case Notation::INFIX : {


            break;
        }
        case Notation::POSTFIX : {


            break;
        }
    }
    return {result};
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
    return std::string(); // todo
}

std::vector<NameMatchInfo>
SpecialConnective::match(const Parser2::LexemeSequence& target, const std::pair<size_t, size_t>& bounds) const {
    return std::vector<NameMatchInfo>();
    // todo использовать matchWithGaps
}

BinaryOperation* cartesian_product = new BinaryOperation(new StringName("\\times"), typeOfTypes, typeOfTypes, typeOfTypes);
BinaryOperation* map_symbol   = new BinaryOperation(new StringName("\\rightarrow"), typeOfTypes, typeOfTypes, typeOfTypes);
