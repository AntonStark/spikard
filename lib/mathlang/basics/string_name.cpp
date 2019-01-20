//
// Created by anton on 20.01.19.
//

#include "string_name.hpp"

void StringName::fromStr(const std::string& name)
{ _name = name; }

std::string StringName::toStr() const
{ return _name; }
