//
// Created by anton on 31.10.18.
//

#include "primary.hpp"

const Terms* Primary::get(Path path) const
{ return (path.empty() ? this : nullptr); }
Terms* Primary::replace(Path path, const Terms* by) const
{ return (path.empty() ? by->clone() : nullptr); }
Terms* Primary::replace(const Terms* x, const Terms* t) const
{ return (comp(x) ? t->clone() : this->clone()); }

MathType* typeOfTypes = new PrimaryMT(new StringName("Type"));
PrimaryMT any_mt(new StringName("any"));
PrimaryMT logical_mt(new StringName("Logical"));

std::string PrimaryMT::print() const
{ return getName(); }
bool PrimaryMT::comp(const Terms* other) const {
    if (auto otherPrimary = dynamic_cast<const PrimaryMT*>(other))
        return (getName() == otherPrimary->getName());
    else
        return false;
}

bool Variable::comp(const Terms* other) const {
    if (auto otherVariable = dynamic_cast<const Variable*>(other))
        return (getType()->comp(otherVariable->getType())
                && *getName() == *otherVariable->getName());
    else
        return false;
}
