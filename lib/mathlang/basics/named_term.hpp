//
// Created by anton on 31.10.18.
//

#ifndef SPIKARD_NAMED_TERM_HPP
#define SPIKARD_NAMED_TERM_HPP

#include "../consepts/abstract_name.hpp"
#include "../consepts/terms.hpp"

class NamedTerm : public virtual Terms
{
private:
    const AbstractName* _name; // todo попробовать unique_pointer
public:
    NamedTerm(const AbstractName* name) : _name(name) {}
    NamedTerm(const NamedTerm& one): _name(one._name) {}
    virtual ~NamedTerm() = default;

    bool operator== (const NamedTerm& one) const
    { return (_name == one._name); }
    bool operator< (const NamedTerm& other) const
    { return (_name < other._name); }

    const AbstractName* const getName() const { return _name; }
    virtual bool comp(const Terms* other) const override;

    const Terms* get(Path path) const override;
    Terms* replace(Path path, const Terms* by) const override;
    Terms* replace(const Terms* x, const Terms* t) const override;
};

/**
 * @brief Класс для описания атомарного терма, переменной
 *
 * // todo поддержка Contraint
 */
class Variable : public NamedTerm
{
private:
    const MathType* _type;
public:
    Variable(AbstractName* name, const MathType* type)
        : NamedTerm(name), _type(type) {}
    Variable(const Variable& one)
        : NamedTerm(one) { _type = one.getType()->clone(); }
    ~Variable() override = default;

    const MathType* getType() const override { return _type; }
    Variable* clone() const override { return new Variable(*this); }
    std::string print() const override { return getName()->toStr(); }
};

#endif //SPIKARD_NAMED_TERM_HPP
