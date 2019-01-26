//
// Created by anton on 20.01.19.
//

#include "function.hpp"

/**
 * для проверки типов аргументов надо проверять является ли argT конструктивным
 * с операцией \times в качестве связки или же простым и тогда проверять размер args
 *
 * значит нужна бинарная операция \times (и ещё \mapsto заодно)
 *
 */
bool Function::comp(const Terms* other) const {
    if (auto otherFunction = dynamic_cast<const Function*>(other))
        return (*getType() == *otherFunction->getType()
                && *getName() == *otherFunction->getName());
    else
        return false;
}
