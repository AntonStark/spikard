//
// Created by anton on 31.10.18.
//

#include "primary.hpp"

const AbstractTerm* PrimaryTerm::get(Path path) const
{ return (path.empty() ? this : nullptr); }
AbstractTerm* PrimaryTerm::replace(Path path, const AbstractTerm* by) const
{ return (path.empty() ? by->clone() : nullptr); }
AbstractTerm* PrimaryTerm::replace(const AbstractTerm* x, const AbstractTerm* t) const
{ return (comp(x) ? t->clone() : this->clone()); }

MathType* typeOfTypes = new PrimaryType(new StringName("Type"));
PrimaryType any_mt(new StringName("any"));
PrimaryType logical_mt(new StringName("Logical"));

std::string PrimaryType::print() const
{ return getName(); }
bool PrimaryType::comp(const AbstractTerm* other) const {
    if (auto otherPrimary = dynamic_cast<const PrimaryType*>(other))
        return (getName() == otherPrimary->getName());
    else
        return false;
}

bool Variable::comp(const AbstractTerm* other) const {
    if (auto otherVariable = dynamic_cast<const Variable*>(other))
        return (getType()->comp(otherVariable->getType())
                && *getName() == *otherVariable->getName());
    else
        return false;
}
