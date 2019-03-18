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

class AbstractTerm
{
public:
    virtual ~AbstractTerm() = default;
    typedef std::stack<size_t> Path;
    typedef std::vector<const AbstractTerm*> Vector;

    virtual const MathType* getType() const = 0;
    virtual bool comp(const AbstractTerm* other) const = 0;

    virtual AbstractTerm* clone() const = 0;
    virtual const AbstractTerm* get(Path path) const = 0;
    virtual AbstractTerm* replace(Path path, const AbstractTerm* by) const = 0;
    virtual AbstractTerm* replace(const AbstractTerm* x, const AbstractTerm* t) const = 0;

    virtual std::string print() const = 0;
};

class MathType : public virtual AbstractTerm
{
public:
    virtual ~MathType() {};
//    virtual bool operator< (const MathType& other) const = 0;

    virtual MathType* clone() const = 0;
    virtual std::string getName() const = 0;
    typedef std::vector<const MathType*> Vector;

//    const MathType* getType() const override;
};

#endif //TEST_BUILD_LOGIC_HPP
