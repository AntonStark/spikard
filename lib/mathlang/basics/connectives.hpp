//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_CONNECTIVES_HPP
#define SPIKARD_CONNECTIVES_HPP

#include "../consepts/abstract_name.hpp"
#include "../consepts/abstract_connective.hpp"

#include "../parser/lexeme.hpp"

#include "complex.hpp"
#include "tex_name.hpp"
#include "argument_place.hpp"

class MatchCheckingConnective {
public:
    virtual std::vector<NameMatchInfo> match(
        const Parser2::LexemeSequence& target,
        const std::pair<size_t, size_t>& bounds) const = 0;
};

class UnaryOperation : public PrintableConnective, public MatchCheckingConnective
{
private:
    const AbstractName* _name;
    bool _prefix;

    const MathType* _operandType;
    const MathType* _resultType;
protected:
    AbstractTerm::Vector compose(AbstractTerm::Vector args) const override
    { return args; }
public:
    UnaryOperation(const AbstractName* name,
                   const MathType* operandType, const MathType* resultType, bool prefix = true);
    ~UnaryOperation() override = default;

    const MathType* resultType() const override
    { return _resultType; }

    bool check(AbstractTerm::Vector args) const override
    { return (args.size() == 1 && _operandType->comp(args.front()->getType())); }

    std::string print(AbstractTerm::Vector args) const override;
    size_t getArity() const override
    { return 1; }

    static const AbstractName* produceSymForm(const AbstractName* ownName, bool prefix);

    std::vector<NameMatchInfo>
    match(const Parser2::LexemeSequence& target, const std::pair<size_t, size_t>& bounds) const override;
};

class BinaryOperation : public PrintableConnective, public MatchCheckingConnective
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
    AbstractTerm::Vector compose(AbstractTerm::Vector args) const override
    { return args; }
public:
    BinaryOperation(const AbstractName* name, const MathType* leftType, const MathType* rightType,
                    const MathType* resultType, Notation notation = Notation::INFIX);
    ~BinaryOperation() override = default;

    const MathType* resultType() const override
    { return _resultType; }

    bool check(AbstractTerm::Vector args) const override;
    std::string print(AbstractTerm::Vector args) const override;
    size_t getArity() const override
    { return 2; }

    static const AbstractName* produceSymForm(const AbstractName* ownName, Notation notation);

    std::vector<NameMatchInfo>
    match(const Parser2::LexemeSequence& target, const std::pair<size_t, size_t>& bounds) const override;
};
extern BinaryOperation* cartesian_product;
extern BinaryOperation* map_symbol;

class SpecialConnective : public PrintableConnective, public MatchCheckingConnective
{
private:
    const AbstractName* _form;
    
    const MathType::Vector _argTypes;
    const MathType* _resultType;
protected:
    AbstractTerm::Vector compose(AbstractTerm::Vector args) const override
    { return args; }
public:
    SpecialConnective(const AbstractName* form,
                      MathType::Vector argT, const MathType* retT) 
        : PrintableConnective(form), _form(form), _argTypes(std::move(argT)), _resultType(retT) {}
    ~SpecialConnective() override = default;

    const MathType* resultType() const override 
    { return _resultType; }

    bool check(AbstractTerm::Vector args) const override;
    std::string print(AbstractTerm::Vector args) const override;
    size_t getArity() const override
    { return _argTypes.size(); }

    std::vector<NameMatchInfo>
    match(const Parser2::LexemeSequence& target, const std::pair<size_t, size_t>& bounds) const override;
};

#endif //SPIKARD_CONNECTIVES_HPP
