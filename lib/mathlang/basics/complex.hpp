//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_COMPLEX_HPP
#define SPIKARD_COMPLEX_HPP

#include "../consepts/terms.hpp"
#include "../consepts/abstract_connective.hpp"

class Complex : public Terms
{
private:
    const AbstractConnective* _symbol;
    const TermsVector _args;
public:
    Complex(const AbstractConnective* symbol, TermsVector args)
        : _symbol(symbol), _args(symbol->compose(args)) {}
    Complex(const Complex& other) = default;

    const AbstractConnective* getSym() const;
    const MathType* getType() const override;
    bool comp(const Terms* other) const override;

    Terms* clone() const override;

    const Terms* arg(size_t oneTwoThree) const;
    const Terms* get(Path path) const override;

    Terms* replace(Path path, const Terms* by) const override;
    Terms* replace(const Terms* x, const Terms* t) const override;

    std::string print() const override;
};

#endif //SPIKARD_COMPLEX_HPP
