//
// Created by anton on 20.01.19.
//

#include "connectives.hpp"

UnaryOperation::UnaryOperation(const AbstractName* name, const MathType* operandType,
                               const MathType* resultType, bool prefix)
    : PrintableConnective(produceSymForm(name, prefix)), _name(name), _operandType(operandType),
      _resultType(resultType), _prefix(prefix) {}
const MathType* UnaryOperation::resultType() const
{ return _resultType; }
bool UnaryOperation::check(TermsVector args) const
{ return (args.size() == 1 && args.front()->getType() == _operandType); }
TermsVector UnaryOperation::compose(TermsVector args) const
{ return args; }
std::string UnaryOperation::print(TermsVector args) const {
    if (!check(args))
        return "";
    else {
        if (_prefix)
            return (_name->toStr() + args.front()->print());
        else
            return (args.front()->print() + _name->toStr());
    }
}
size_t UnaryOperation::getArity() const
{ return 1; }

const AbstractName* UnaryOperation::produceSymForm(const AbstractName* ownName, bool prefix) {
    auto name = ownName->toStr();
    if (prefix)
        return new TexName(name + "\\cdot");
    else
        return new TexName("\\cdot" + name);
}

BinaryOperation::BinaryOperation(const AbstractName* name, const MathType* leftType, const MathType* rightType,
                                 const MathType* resultType, BinaryOperation::Notation notation)
    : PrintableConnective(produceSymForm(name, notation)), _name(name), _leftType(leftType), _rightType(rightType),
      _resultType(resultType), _notation(notation) {}
const MathType* BinaryOperation::resultType() const
{ return _resultType; }
bool BinaryOperation::check(TermsVector args) const {
    return (args.size() == 2
            && args.at(0)->getType() == _leftType
            && args.at(1)->getType() == _rightType);
}
TermsVector BinaryOperation::compose(TermsVector args) const
{ return args; }
std::string BinaryOperation::print(TermsVector args) const {
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
size_t BinaryOperation::getArity() const
{ return 2; }

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

