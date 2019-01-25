//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_FUNCTION_HPP
#define SPIKARD_FUNCTION_HPP

#include "named_term.hpp"
#include "connectives.hpp"

/**
 * @brief Класс описывает обычные функции вроде f(x), g(x, y)
 *
 * Реализуются функции одного аргумента.
 * g(x, y) понимается как g от упорядоченной пары x, y
 */
class Function : public NamedTerm, public UnaryOperation
{
    const MathType* _mapType;
public:
    Function(const AbstractName* name, const MathType* argT, const MathType* retT)
        : NamedTerm(name), UnaryOperation(name, argT, retT, false) {}
    Function(const AbstractName* name, const MathType* mapMT);

};

#endif //SPIKARD_FUNCTION_HPP
