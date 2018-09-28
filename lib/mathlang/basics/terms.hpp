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

class NamedTerm : public Terms
{
private:
    std::string _name;
public:
    NamedTerm(std::string name) : _name(std::move(name)) {}
    NamedTerm(const NamedTerm& one): _name(one._name) {}
    virtual ~NamedTerm() = default;

    bool operator== (const NamedTerm& one) const
    { return (_name == one._name); }
    bool operator< (const NamedTerm& other) const
    { return (_name < other._name); }

    std::string getName() const { return _name; }
    void setName(const std::string& name) { _name = name; }
    virtual bool comp(const Terms* other) const override;

    const Terms* get(Path path) const override;
    Terms* replace(Path path, const Terms* by) const override;
    Terms* replace(const Terms* x, const Terms* t) const override;
};

class Constant : public NamedTerm
{
private:
    const MathType* _type;
public:
    Constant(std::string name, const MathType* type)
        : NamedTerm(name), _type(type) {}
    Constant(const Constant& one)
        : NamedTerm(one.getName()) { _type = one.getType()->clone(); }
    ~Constant() override = default;

    const MathType* getType() const override { return _type; }
    Constant* clone() const override { return new Constant(*this); }
    std::string print() const override { return getName(); }
};

class Variable : public NamedTerm
{
private:
    const MathType* _type;
public:
    Variable(std::string name, const MathType* type)
            : NamedTerm(name), _type(type) {}
    Variable(const Variable& one)
        : NamedTerm(one.getName()) { _type = one.getType()->clone(); }
    ~Variable() override = default;

    const MathType* getType() const override { return _type; }
    Variable* clone() const override { return new Variable(*this); }
    std::string print() const override { return getName(); }
};

#endif //TEST_BUILD_LOGIC_HPP
