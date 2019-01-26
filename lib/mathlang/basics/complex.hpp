#include <utility>

//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_COMPLEX_HPP
#define SPIKARD_COMPLEX_HPP

#include "../consepts/terms.hpp"
#include "../consepts/abstract_connective.hpp"

#include "mathtype.hpp"

class Complex : public Terms
{
private:
    const AbstractConnective* _symbol;
    const Terms::Vector _args;
public:
    Complex(const AbstractConnective* symbol, Terms::Vector args)
        : _symbol(symbol), _args(symbol->apply(std::move(args))) {}
    Complex(const Complex& other) = default;

    const AbstractConnective* getSym() const
    { return _symbol; }
    const MathType* getType() const override
    { return _symbol->resultType(); }
    bool comp(const Terms* other) const override;

    Terms* clone() const override
    { return new Complex(*this); }

    const Terms* arg(size_t oneTwoThree) const
    { return _args.at(oneTwoThree-1); }
    const Terms* get(Path path) const override;

    Terms* replace(Path path, const Terms* by) const override;
    Terms* replace(const Terms* x, const Terms* t) const override
    { return nullptr; } // fixme заглушка, убрать вовсе

    std::string print() const override
    { _symbol->print(_args); }
};

class ComplexMT : public MathType, public Complex
{
public:
    ComplexMT(const AbstractConnective* symbol, Terms::Vector args)
        : Complex(symbol, args) {}
    ~ComplexMT() override = default;

    bool isPrimary() const override { return false; }
    bool operator==(const MathType& other) const override;
    bool operator<(const MathType& other) const override;

    std::string getName() const override
    { return print(); }

    const MathType* getType() const override
    { return typeOfTypes; }
    ComplexMT* clone() const override
    { return new ComplexMT(*this); }
};

#endif //SPIKARD_COMPLEX_HPP
