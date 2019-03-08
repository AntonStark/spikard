//
// Created by anton on 02.03.19.
//

#ifndef SPIKARD_ARGUMENT_PLACE_HPP
#define SPIKARD_ARGUMENT_PLACE_HPP

#include "../consepts/terms.hpp"
#include "../consepts/abstract_name.hpp"

typedef std::pair<size_t, size_t> ElemBounds;

struct NameArgInfo
{
    ElemBounds bounds;
    bool nameExpected;
    const MathType* _type;
    NameArgInfo(size_t from, size_t to, bool name, const MathType* type)
        : bounds(std::make_pair(from, to)), nameExpected(name), _type(type) {}
};

/// Отвечает за описание границ имени и его аргументных мест
/// Внимание: сейчас считается, что необработанного участка строки не остаётся (нет узлов сочинения)
struct NameMatchInfo
{
    const AbstractName* _name;
    bool _varPlaces;
    std::vector<NameArgInfo> _args;

    explicit NameMatchInfo(const AbstractName* name)
        : _name(name), _varPlaces(false) {}
    void add(size_t from, size_t to, const MathType* type, bool isVarPlace = false);
    bool hasVarPlaces() const
    { return _varPlaces; }
};

#endif //SPIKARD_ARGUMENT_PLACE_HPP
