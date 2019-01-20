//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_COMPLEX_HPP
#define SPIKARD_COMPLEX_HPP

#include "terms.hpp"
#include "abstract_complificator.hpp"

class Complex : public Terms
{
public:
    typedef AbstractComplificator::TermsVector TermsVector;
private:
    const AbstractComplificator* _symbol;
    const TermsVector _args;
public:
    Complex(AbstractComplificator* symbol, const TermsVector& args)
        : Complex(*symbol->compose(args)) {}
    Complex(const AbstractComplificator* symbol, TermsVector args)
        : _symbol(symbol), _args(std::move(args)) {}
    Complex(const Complex& other) = default;

    const AbstractComplificator* getSym() const
    { return _symbol; }
    const MathType* getType() const override
    { return _symbol->resultType(); }
};

#endif //SPIKARD_COMPLEX_HPP
