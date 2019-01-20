//
// Created by anton on 20.01.19.
//

#include "connectives.hpp"

const MathType* UnaryOperation::resultType() const
{ return _resultType; }
bool UnaryOperation::check(AbstractConnective::TermsVector args) const
{ return (args.size() == 1 && args.front()->getType() == _operandType); }
Complex* UnaryOperation::compose(AbstractConnective::TermsVector args) const {
    if (check(args))
        return new Complex(this, args);
    else
        return nullptr;
}
std::string UnaryOperation::print(AbstractConnective::TermsVector args) const {
    if (!check(args))
        return "";
    else {
        if (_prefix)
            return (_name->toStr() + args.front()->print());
        else
            return (args.front()->print() + _name->toStr());
    }
}

const MathType* BinaryOperation::resultType() const
{ return _resultType; }

bool BinaryOperation::check(AbstractConnective::TermsVector args) const {
    return (args.size() == 2
            && args.at(0)->getType() == _leftType
            && args.at(1)->getType() == _rightType);
}

Complex* BinaryOperation::compose(AbstractConnective::TermsVector args) const {
    if (check(args))
        return new Complex(this, args);
    else
        return nullptr;
}

std::string BinaryOperation::print(AbstractConnective::TermsVector args) const {
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
