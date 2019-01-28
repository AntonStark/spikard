//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_FUNCTION_HPP
#define SPIKARD_FUNCTION_HPP

#include "primary.hpp"
#include "connectives.hpp"

/**
 * @brief Класс описывает обычные функции вроде f(x), g(x, y)
 *
 * Реализуются функции одного аргумента.
 * g(x, y) понимается как g от упорядоченной пары x, y
 */
class Function : public Primary, public UnaryOperation
{
    const MathType* _mapType;
public:
    Function(const AbstractName* name, const MathType* argT, const MathType* retT)
        : UnaryOperation(name, argT, retT)/*, _mapType(new ComplexMT(map_symbol, {}))*/ {}
//    Function(const AbstractName* name, const MathType* mapMT)
    ~Function() override = default;

    const MathType* getType() const override
    { return /*_mapType*/resultType(); } // fixme заглушка
    bool comp(const Terms* other) const override;
    Function* clone() const override
    { return new Function(*this); }
    std::string print() const override
    { return getName()->toStr(); }
};

#endif //SPIKARD_FUNCTION_HPP
