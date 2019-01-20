//
// Created by anton on 21.01.19.
//

#ifndef SPIKARD_ABSTRACT_COMPLIFICATOR_HPP
#define SPIKARD_ABSTRACT_COMPLIFICATOR_HPP

#include "abstract_connective.hpp"
#include "complex.hpp"

class AbstractComplificator : public AbstractConnective
{
public:
    using AbstractConnective::TermsVector;
    virtual Complex* compose(TermsVector args) const = 0;
};

#endif //SPIKARD_ABSTRACT_COMPLIFICATOR_HPP
