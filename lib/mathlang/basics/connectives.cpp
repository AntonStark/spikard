//
// Created by anton on 20.01.19.
//

#include "connectives.hpp"

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
