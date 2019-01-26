//
// Created by anton on 17.05.18.
//

#ifndef SPIKARD_MATHTYPE_HPP
#define SPIKARD_MATHTYPE_HPP

#include <string>
#include <sstream>
#include <vector>
#include <typeinfo>

#include "../consepts/terms.hpp"

#include "../parser/lexer.hpp"

#include "named_term.hpp"
#include "string_name.hpp"

class PrimaryMT : public MathType, public NamedEntity, public Primary
{
public:
    PrimaryMT(const AbstractName* typeName) : NamedEntity(typeName) {}
    PrimaryMT(const PrimaryMT&) = default;
    ~PrimaryMT() override = default;

    bool operator== (const MathType& other) const override;
    bool operator<(const MathType& other) const override;
    bool isPrimary() const override { return true; }

    bool comp(const Terms* other) const override;

    PrimaryMT* clone() const override { return new PrimaryMT(*this); }
    std::string getName() const override { return NamedEntity::getName()->toStr(); }

    const MathType* getType() const override;

    std::string print() const override;
};

extern MathType* typeOfTypes;
extern PrimaryMT any_mt;
extern PrimaryMT logical_mt;

class ProductMT : public MathType
{
private:
    MTVector _subTypes;
public:
    ProductMT(MTVector subTypes) : _subTypes(subTypes) {};
    ProductMT(const ProductMT&) = default;
    ~ProductMT() override = default;

    bool operator==(const MathType& one) const override;
    bool operator<(const MathType& other) const override;
    bool isPrimary() const override { return false; }

    MathType* clone() const override;
    std::string getName() const override;
    std::vector<std::string> getNames() const;
    size_t getArity() const { return _subTypes.size(); }
    bool matchArgType(const MTVector& otherMTV) const;
};

class MapMT : public MathType
{
private:
    const ProductMT* _argsT;
    const MathType* _retT;
public:
    MapMT(const ProductMT& argsT, const MathType* retT) : _retT(retT)
    { _argsT = new ProductMT(argsT); }
    MapMT(const MapMT& one) : _retT(one._retT)
    { _argsT = new ProductMT(*one._argsT); }
    ~MapMT() { delete _argsT; }

    bool operator==(const MathType& one) const override;
    bool operator<(const MathType& other) const override;
    bool isPrimary() const override { return false; }

    MathType* clone() const override { return new MapMT(*this); }
    std::string getName() const override;
    size_t getArity() const { return _argsT->getArity(); }
    const ProductMT* getArgs() const { return _argsT; }
    const MathType* getRet() const { return _retT; }
};

#endif //SPIKARD_MATHTYPE_HPP
