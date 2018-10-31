//
// Created by anton on 28.09.18.
//

#ifndef SPIKARD_MATHLANG_MAPTERM_HPP
#define SPIKARD_MATHLANG_MAPTERM_HPP

#include "named_term.hpp"

// Это реализация неоднородного символа.
// Подходит для символов малой арности, но не каких-нубудь R^n->R^m
// Отображения сами являются термами, поскольку есть отображения отображений.
// Например, символ взятия производной.

struct ArgForm
{
    bool prefix;
    bool top;
    bool bottom;
    size_t tail;
};

class Map : public NamedTerm
{
public:
    typedef MathType::MTVector MTVector;
    class argN_oper_error;
private:
    MapMT _type;
public:
    Map(const std::string& name, ProductMT argT, const MathType* retT);
    Map(const std::string& name, size_t arity,
        const MathType* argT, const MathType* retT)
        : Map(name, ProductMT({arity, argT}), retT) {}
    Map(const Map&) = default;
    ~Map() override = default;

    bool operator<(const Map& other) const;

    size_t getArity() const { return _type.getArity(); }
    const MathType* getType() const override { return _type.getRet(); }
    const ProductMT* getArgs() const { return _type.getArgs(); }
    bool matchArgType(const MTVector& otherArgT) const
    { return _type.getArgs()->matchArgType(otherArgT); }

    Terms* clone() const override { return new Map(*this); }
    std::string print() const { return getName(); }

    static ArgForm parseForm(std::string symDefStr);
    static Map* create(std::string symForm, const ProductMT& argT, const MathType* retT);
};

class UnaryOperation : public Map
{
public:
    enum class Form {PRE, POST, TOP, BOT};
private:
    Form _form;
public:
    UnaryOperation(const std::string& name, Form form,
                   ProductMT argT, const MathType* retT);
    Terms* clone() const override { return new UnaryOperation(*this); }
};

class BinaryOperation : public Map
{
public:
    enum class Form {PREF_TOP, PREF_BOT, BOT_TOP,
        BOT_MID, TOP_MID, INFIX, FOLLOW};
private:
    Form _form;
public:
    BinaryOperation(const std::string& name, Form form,
                    ProductMT argT, const MathType* ret);
    Terms* clone() const override { return new BinaryOperation(*this); }
};

class TernaryOperation : public Map
{
public:
    TernaryOperation(const std::string& name, ProductMT argT, const MathType* ret);
    Terms* clone() const override { return new TernaryOperation(*this); }
};

#endif //SPIKARD_MATHLANG_MAPTERM_HPP
