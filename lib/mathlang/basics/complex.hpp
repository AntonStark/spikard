//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_COMPLEX_HPP
#define SPIKARD_COMPLEX_HPP

#include "../consepts/terms.hpp"
#include "../consepts/abstract_connective.hpp"

#include "primary.hpp"

class ComplexTerm : public virtual AbstractTerm
{
private:
    const AbstractConnective* _symbol;
    const AbstractTerm::Vector _args;
public:
    ComplexTerm(const AbstractConnective* symbol, AbstractTerm::Vector args)
        : _symbol(symbol), _args(symbol->apply(std::move(args))) {}
    ComplexTerm(const ComplexTerm& other) = default;

    const AbstractConnective* getSym() const
    { return _symbol; }
    const MathType* getType() const override
    { return _symbol->resultType(); }
    bool comp(const AbstractTerm* other) const override;

    AbstractTerm* clone() const override
    { return new ComplexTerm(*this); }

    const AbstractTerm* arg(size_t oneTwoThree) const
    { return _args.at(oneTwoThree-1); }
    const AbstractTerm* get(Path path) const override;

    AbstractTerm* replace(Path path, const AbstractTerm* by) const override;
    AbstractTerm* replace(const AbstractTerm* x, const AbstractTerm* t) const override
    { return nullptr; } // fixme заглушка, убрать вовсе

    std::string print() const override
    { return _symbol->print(_args); }
};

class ComplexType : public MathType, public ComplexTerm
{
public:
    ComplexType(const AbstractConnective* symbol, AbstractTerm::Vector args)
        : ComplexTerm(symbol, args) {}
    ~ComplexType() override = default;

    const MathType* getType() const override
    { return typeOfTypes; }

    std::string getName() const override
    { return print(); }
    ComplexType* clone() const override
    { return new ComplexType(*this); }
};

#endif //SPIKARD_COMPLEX_HPP
