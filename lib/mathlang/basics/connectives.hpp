//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_CONNECTIVES_HPP
#define SPIKARD_CONNECTIVES_HPP

#include "../consepts/abstract_name.hpp"
#include "../consepts/abstract_complificator.hpp"

#include "../consepts/complex.hpp"

class UnaryOperation : public AbstractComplificator
{
private:
    const MathType* _operandType;
    const MathType* _resultType;

    const AbstractName* _name;
    bool _prefix;
public:
    UnaryOperation(const MathType* operandType, const MathType* resultType,
                   const AbstractName* name, bool prefix = true)
        : _operandType(operandType), _resultType(resultType),
          _name(name), _prefix(prefix) {}

    const MathType* resultType() const override;
    bool check(TermsVector args) const override;
    Complex* compose(TermsVector args) const override;
    std::string print(TermsVector args) const override;
};

class BinaryOperation : public AbstractComplificator
{
public:
    enum class Notation {PREFIX, INFIX, POSTFIX};
private:
    const MathType* _leftType;
    const MathType* _rightType;
    const MathType* _resultType;

    const AbstractName* _name;
    Notation _notation;
public:
    BinaryOperation(const MathType* leftType, const MathType* rightType, const MathType* resultType,
                    const AbstractName* name, Notation notation = Notation::INFIX)
        : _leftType(leftType), _rightType(rightType), _resultType(resultType),
          _name(name), _notation(notation) {}

    const MathType* resultType() const override;
    bool check(TermsVector args) const override;
    Complex* compose(TermsVector args) const override;
    std::string print(TermsVector args) const override;
};

#endif //SPIKARD_CONNECTIVES_HPP
