//
// Created by anton on 31.10.18.
//

#ifndef SPIKARD_NAMED_TERM_HPP
#define SPIKARD_NAMED_TERM_HPP

#include "../consepts/abstract_name.hpp"
#include "../consepts/terms.hpp"

#include "string_name.hpp"

extern MathType* typeOfTypes;

class PrimaryTerm : public virtual AbstractTerm
{
public:
    const AbstractTerm* get(Path path) const override;
    AbstractTerm* replace(Path path, const AbstractTerm* by) const override;
    AbstractTerm* replace(const AbstractTerm* x, const AbstractTerm* t) const override;
};

class PrimaryType : public NamedEntity, public MathType, public PrimaryTerm
{
public:
    PrimaryType(const AbstractName* typeName) : NamedEntity(typeName) {}
    PrimaryType(const PrimaryType&) = default;
    ~PrimaryType() override = default;

    bool isPrimary() const override
    { return true; }
    const MathType* getType() const override
    { return typeOfTypes; }
    bool comp(const AbstractTerm* other) const override;

    PrimaryType* clone() const override
    { return new PrimaryType(*this); }
    std::string getName() const override
    { return NamedEntity::getName()->toStr(); }

    std::string print() const override;
};

extern PrimaryType any_mt;
extern PrimaryType logical_mt;

/**
 * @brief Класс для описания атомарного терма, переменной
 */
class Variable : public NamedEntity, public PrimaryTerm
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
    bool comp(const AbstractTerm* other) const override;
    Variable* clone() const override { return new Variable(*this); }
    std::string print() const override { return getName()->toStr(); }
};

#endif //SPIKARD_NAMED_TERM_HPP
