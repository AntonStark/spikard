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

bool PrimaryMT::operator==(const MathType& one) const {
    if (getName() == "any")
        return true;
    if (one.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(one);
        return (pmt.getName() == "any" || getName() == pmt.getName());
    }
    else
        return false;
}
bool PrimaryMT::operator<(const MathType& other) const {
    if (other.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(other);
        return (getName() < pmt.getName());
    }
    else
        return true;
}
const MathType* PrimaryMT::getType() const
{ return typeOfTypes; }
std::string PrimaryMT::print() const
{ return getName(); }
bool PrimaryMT::comp(const Terms* other) const {
    if (auto otherPrimary = dynamic_cast<const PrimaryMT*>(other))
        return (*getType() == *otherPrimary->getType()
                && getName() == otherPrimary->getName());
    else
        return false;
}

bool Variable::comp(const Terms* other) const {
    if (auto otherVariable = dynamic_cast<const Variable*>(other))
        return (*getType() == *otherVariable->getType()
                && *getName() == *otherVariable->getName());
    else
        return false;
}
