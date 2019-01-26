//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_CONNECTIVES_HPP
#define SPIKARD_CONNECTIVES_HPP

#include "../consepts/abstract_name.hpp"
#include "../consepts/abstract_connective.hpp"

#include "complex.hpp"
#include "texname.hpp"

class PrintableConnective : public AbstractConnective, public NamedEntity
{
public:
    PrintableConnective(const AbstractName* name) : NamedEntity(name) {}
    ~PrintableConnective() override = default;
};

class UnaryOperation : public PrintableConnective
{
private:
    const AbstractName* _name;
    bool _prefix;

    const MathType* _operandType;
    const MathType* _resultType;
protected:
    TermsVector compose(TermsVector args) const override;
public:
    UnaryOperation(const AbstractName* name,
                   const MathType* operandType, const MathType* resultType, bool prefix = true);

    const MathType* resultType() const override;
    bool check(TermsVector args) const override;
    std::string print(TermsVector args) const override;
    size_t getArity() const override;

    static const AbstractName* produceSymForm(const AbstractName* ownName, bool prefix);
};

class BinaryOperation : public PrintableConnective
{
public:
    enum class Notation {PREFIX, INFIX, POSTFIX};
private:
    const AbstractName* _name;
    Notation _notation;

    const MathType* _leftType;
    const MathType* _rightType;
    const MathType* _resultType;
protected:
    TermsVector compose(TermsVector args) const override;
public:
    BinaryOperation(const AbstractName* name, const MathType* leftType, const MathType* rightType,
                    const MathType* resultType, Notation notation = Notation::INFIX);

    const MathType* resultType() const override;
    bool check(TermsVector args) const override;
    std::string print(TermsVector args) const override;
    size_t getArity() const override;

    static const AbstractName* produceSymForm(const AbstractName* ownName, Notation notation);
};
extern BinaryOperation* cartesian_product;
extern BinaryOperation* map_symbol;

/*class SpecialConnective : public PrintableConnective
{

};*/

#endif //SPIKARD_CONNECTIVES_HPP
