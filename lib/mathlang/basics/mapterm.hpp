//
// Created by anton on 28.09.18.
//

#ifndef SPIKARD_MATHLANG_MAPTERM_HPP
#define SPIKARD_MATHLANG_MAPTERM_HPP

#include "../parser/lexeme.hpp"

#include "named_term.hpp"

// Это реализация неоднородного символа.
// Подходит для символов малой арности, но не каких-нубудь R^n->R^m
// Отображения сами являются термами, поскольку есть отображения отображений.
// Например, символ взятия производной.

class Map : public NamedTerm
{
public:
    typedef MathType::MTVector MTVector;
private:
    MapMT _type;
public:
    Map(const NamesType& symForm, ProductMT argT, const MathType* retT);
    Map(const NamesType& symForm, size_t arity,
        const MathType* argT, const MathType* retT)
        : Map(symForm, ProductMT({arity, argT}), retT) {}
    Map(const Map&) = default;
    ~Map() override = default;

    bool operator<(const Map& other) const;

    size_t getArity() const { return _type.getArity(); }
    const MathType* getType() const override { return _type.getRet(); }
    const ProductMT* getArgs() const { return _type.getArgs(); }
    bool matchArgType(const MTVector& otherArgT) const
    { return _type.getArgs()->matchArgType(otherArgT); }

    Terms* clone() const override { return new Map(*this); }
    std::string print() const { return getName(); }
};

#endif //SPIKARD_MATHLANG_MAPTERM_HPP
