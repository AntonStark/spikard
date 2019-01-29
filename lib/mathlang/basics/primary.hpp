//
// Created by anton on 31.10.18.
//

#ifndef SPIKARD_NAMED_TERM_HPP
#define SPIKARD_NAMED_TERM_HPP

#include "../consepts/abstract_name.hpp"
#include "../consepts/terms.hpp"

#include "string_name.hpp"

extern MathType* typeOfTypes;

class Primary : public virtual Terms
{
public:
    const Terms* get(Path path) const override;
    Terms* replace(Path path, const Terms* by) const override;
    Terms* replace(const Terms* x, const Terms* t) const override;
};

class PrimaryMT : public NamedEntity, public MathType, public Primary
{
public:
    PrimaryMT(const AbstractName* typeName) : NamedEntity(typeName) {}
    PrimaryMT(const PrimaryMT&) = default;
    ~PrimaryMT() override = default;

    bool isPrimary() const override
    { return true; }
    const MathType* getType() const override
    { return typeOfTypes; }
    bool comp(const Terms* other) const override;

    PrimaryMT* clone() const override
    { return new PrimaryMT(*this); }
    std::string getName() const override
    { return NamedEntity::getName()->toStr(); }

    std::string print() const override;
};

extern PrimaryMT any_mt;
extern PrimaryMT logical_mt;

/**
 * @brief Класс для описания атомарного терма, переменной
 */
class Variable : public NamedEntity, public Primary
{
private:
    const MathType* _type;
public:
    Variable(AbstractName* name, const MathType* type)
        : NamedEntity(name), _type(type) {}
    Variable(const Variable& one)
        : NamedEntity(one) { _type = one.getType()->clone(); }
    ~Variable() override = default;

    const MathType* getType() const override { return _type; }
    bool comp(const Terms* other) const override;
    Variable* clone() const override { return new Variable(*this); }
    std::string print() const override { return getName()->toStr(); }
};

#endif //SPIKARD_NAMED_TERM_HPP
