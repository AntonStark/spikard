//
// Created by anton on 31.10.18.
//

#include "named_term.hpp"

const Terms* Primary::get(Path path) const
{ return (path.empty() ? this : nullptr); }
Terms* Primary::replace(Path path, const Terms* by) const
{ return (path.empty() ? by->clone() : nullptr); }
Terms* Primary::replace(const Terms* x, const Terms* t) const
{ return (comp(x) ? t->clone() : this->clone()); }

bool Variable::comp(const Terms* other) const {
    if (auto otherVariable = dynamic_cast<const Variable*>(other))
        return (*getType() == *otherVariable->getType()
                && *getName() == *otherVariable->getName());
    else
        return false;
}
