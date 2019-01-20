//
// Created by anton on 13.01.19.
//

#ifndef SPIKARD_ABSTRACTCONNECTIVE_HPP
#define SPIKARD_ABSTRACTCONNECTIVE_HPP


#include "terms.hpp"

class AbstractConnective
{
public:
    typedef std::vector<Terms*> TermsVector;
    virtual bool check(TermsVector args) const = 0;
    virtual Terms* compose(TermsVector args) const = 0;
    virtual const MathType* resultType() const = 0;
    virtual std::string print(TermsVector args) const = 0;
};


#endif //SPIKARD_ABSTRACTCONNECTIVE_HPP
