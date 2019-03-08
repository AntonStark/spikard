//
// Created by anton on 02.03.19.
//

#include "argument_place.hpp"

void NameMatchInfo::add(size_t from, size_t to, const MathType* type, bool isVarPlace) {
    _args.emplace_back(from, to, isVarPlace, type);
    if (isVarPlace)
        _varPlaces = true;
}
