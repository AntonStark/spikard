//
// Created by anton on 13.01.19.
//

#ifndef SPIKARD_ABSTRACTCONNECTIVE_HPP
#define SPIKARD_ABSTRACTCONNECTIVE_HPP


#include "terms.hpp"
#include "abstract_name.hpp"

class AbstractConnective
{
protected:
    virtual AbstractTerm::Vector compose(AbstractTerm::Vector args) const = 0;
public:
    virtual ~AbstractConnective() = default;
    virtual size_t getArity() const = 0;
    virtual bool check(AbstractTerm::Vector args) const = 0;
    AbstractTerm::Vector apply(AbstractTerm::Vector args) const {
        if (check(args))
            return compose(args);
        else
            throw std::invalid_argument("");
    }
    virtual const MathType* resultType() const = 0;
    virtual std::string print(AbstractTerm::Vector args) const = 0;
};

class PrintableConnective : public AbstractConnective, public NamedEntity
{
public:
    PrintableConnective(const AbstractName* name) : NamedEntity(name) {}
    ~PrintableConnective() override = default;
};

#endif //SPIKARD_ABSTRACTCONNECTIVE_HPP
