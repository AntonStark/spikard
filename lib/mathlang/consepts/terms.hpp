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

class MathType;

class Terms
{
public:
    virtual ~Terms() = default;
    typedef std::stack<size_t> Path;
    typedef std::vector<const Terms*> Vector;

    virtual const MathType* getType() const = 0;
    virtual bool comp(const Terms* other) const = 0;

    virtual Terms* clone() const = 0;
    virtual const Terms* get(Path path) const = 0;
    virtual Terms* replace(Path path, const Terms* by) const = 0;
    virtual Terms* replace(const Terms* x, const Terms* t) const = 0;

    virtual std::string print() const = 0;
};

class MathType : public virtual Terms
{
public:
    virtual ~MathType() {};
    virtual bool isPrimary() const = 0;
//    virtual bool operator== (const MathType& other) const = 0;
//    bool operator!= (const MathType& other) const
//    { return !(*this == other); }
//    virtual bool operator< (const MathType& other) const = 0;

    virtual MathType* clone() const = 0;
    virtual std::string getName() const = 0;
    typedef std::vector<const MathType*> Vector;

//    const MathType* getType() const override;
//    std::string print() const override;

    // fixme временные заглушки
//    const Terms* get(Path path) const override;
//    Terms* replace(Path path, const Terms* by) const override;
//    Terms* replace(const Terms* x, const Terms* t) const override;
};

#endif //TEST_BUILD_LOGIC_HPP
