//
// Created by anton on 03.12.16.
//

#ifndef TEST_BUILD_LOGIC_HPP
#define TEST_BUILD_LOGIC_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <set>
#include <map>
#include <functional>
#include <utility>
#include <vector>
#include <stack>

#include "mathtype.hpp"

class Terms
{
public:
    typedef std::stack<size_t> Path;

    virtual const MathType* getType() const = 0;
    virtual bool comp(const Terms* other) const = 0;

    virtual Terms* clone() const = 0;
    virtual const Terms* get(Path path) const = 0;
    virtual Terms* replace(Path path, const Terms* by) const = 0;
    virtual Terms* replace(const Terms* x, const Terms* t) const = 0;

    virtual std::string print() const = 0;
};

#endif //TEST_BUILD_LOGIC_HPP