//
// Created by anton on 31.10.18.
//

#ifndef SPIKARD_NAMED_TERM_HPP
#define SPIKARD_NAMED_TERM_HPP

#include "terms.hpp"

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

#endif //SPIKARD_NAMED_TERM_HPP