//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_CONNECTIVES_HPP
#define SPIKARD_CONNECTIVES_HPP

#include "../consepts/abstract_name.hpp"
#include "../consepts/abstract_connective.hpp"

#include "complex.hpp"

class UnaryOperation : public AbstractConnective
{
private:
    const AbstractName* _name;
    bool _prefix;

    const MathType* _operandType;
    const MathType* _resultType;
public:
    UnaryOperation(const AbstractName* name,
                   const MathType* operandType, const MathType* resultType, bool prefix = true);

    const MathType* resultType() const override;
    bool check(TermsVector args) const override;
    TermsVector compose(TermsVector args) const override;
    std::string print(TermsVector args) const override;
    size_t getArity() const override;
};

class BinaryOperation : public AbstractConnective
{
public:
    enum class Notation {PREFIX, INFIX, POSTFIX};
private:
    const AbstractName* _name;
    Notation _notation;

    const MathType* _leftType;
    const MathType* _rightType;
    const MathType* _resultType;
public:
    BinaryOperation(const AbstractName* name, const MathType* leftType, const MathType* rightType,
                    const MathType* resultType, Notation notation = Notation::INFIX);

    const MathType* resultType() const override;
    bool check(TermsVector args) const override;
    TermsVector compose(TermsVector args) const override;
    std::string print(TermsVector args) const override;

    size_t getArity() const override;
};

#endif //SPIKARD_CONNECTIVES_HPP
