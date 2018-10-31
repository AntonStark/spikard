//
// Created by anton on 31.10.18.
//

#include "named_term.hpp"

bool NamedTerm::comp(const Terms* other) const {
    if (auto namedOther = dynamic_cast<const NamedTerm*>(other))
        return (*getType() == *namedOther->getType()
                && getName() == namedOther->getName());
    else
        return false;
}

const Terms* NamedTerm::get(Path path) const
{ return (path.empty() ? this : nullptr); }

Terms* NamedTerm::replace(Path path, const Terms* by) const
{ return (path.empty() ? by->clone() : nullptr); }

Terms* NamedTerm::replace(const Terms* x, const Terms* t) const
{ return (comp(x) ? t->clone() : this->clone()); }