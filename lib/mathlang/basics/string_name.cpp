//
// Created by anton on 20.01.19.
//

#include "string_name.hpp"

void StringName::fromStr(const std::string& name)
{ _name = name; }
std::string StringName::toStr() const
{ return _name; }
bool StringName::operator==(const AbstractName& other) const {
    try {
        auto otherStringName = dynamic_cast<const StringName&>(other);
        return (_name == otherStringName._name);
    } catch (std::bad_cast&)
    { return false; }
}
