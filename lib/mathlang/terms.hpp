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
    NamedTerm(const NamedTerm& one) = default;
    virtual ~NamedTerm() = default;

    bool operator== (const NamedTerm& one) const
    { return (_name == one._name); }
    bool operator< (const NamedTerm& other) const
    { return (_name < other._name); }

    std::string getName() const { return _name; }
    void setName(std::string name) { _name = std::move(name); }
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

// Указание типов в отображнеии вовлекает семантику, но позволяет более полное описание сущности
// Важно, что с априорной информацией о типах упрощается парсер.
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
    Map(std::string name, MTVector argT, const MathType* retT)
        : NamedTerm(std::move(name)), _type(argT, retT) {}
    Map(std::string name, size_t arity, const MathType* argT, const MathType* retT)
        : NamedTerm(std::move(name)), _type({arity, argT}, retT) {}
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

    static std::string extractName(std::string symDefStr);
    static ArgForm parseForm(std::string symDefStr);
    static Map* create(std::string symForm, MTVector argT, const MathType* retT);
};

class UnaryOperation : public Map
{
public:
    enum class Form {PRE, POST, TOP, BOT};
private:
    Form _form;
public:
    UnaryOperation(std::string name, Form form,
                   MTVector argT, const MathType* retT);
    Terms* clone() const override { return new UnaryOperation(*this); }
};

class BinaryOperation : public Map
{
public:
    enum class Form {PREF_TOP, PREF_BOT, BOT_TOP, BOT_MID, TOP_MID, INFIX, FOLLOW};
private:
    Form _form;
public:
    BinaryOperation(std::string name, Form form,
                    MTVector argT, const MathType* ret);
    Terms* clone() const override { return new BinaryOperation(*this); }
};

class TernaryOperation : public Map
{
public:
    TernaryOperation(std::string name, MTVector argT, const MathType* ret);
    Terms* clone() const override { return new TernaryOperation(*this); }
};

class ParenSymbol
{
public:
    typedef std::vector<Terms*> TermsVector;
    class argN_argType_error;
protected:
    // Внимание! ParenSymbol владеет своими аргументами,
    // к передаваемым указателям применяется глубокое копирование
    TermsVector _args;
    /*std::set<Variable> vars;*/
    void checkArgs(const Map& f, TermsVector args) const;
    const TermsVector& replace(Terms::Path path, const Terms* by) const;
public:
    // В обоих случаях применяется глубокое копирование
    ParenSymbol(const TermsVector& args);
    ParenSymbol(const ParenSymbol& one);
    virtual ~ParenSymbol();

    bool operator==(const ParenSymbol& other) const;

    const Terms* arg(size_t oneTwoThree) const
    { return _args.at(oneTwoThree-1); }
    std::string print() const;
};
class Term : public Terms, public ParenSymbol
{
protected:
    void boundVar(Variable var);
    std::string printQ() const;
public:
    typedef std::set<Variable> VarSet;
    enum class QType {FORALL, EXISTS};
    static std::map<QType, const std::string> qword;

    Map _f;
    VarSet free;
    Term(Map f, TermsVector _args);
    Term(std::set<Map> symSet, TermsVector args);
    Term(const Term& one) = default;
    ~Term() override = default;

    size_t getArity() const { return  _f.getArity(); }
    const MathType* getType() const override { return _f.getType(); }
    Map getSym() const { return _f; }
    bool comp(const Terms* other) const override;

    Term* clone() const override { return new Term(*this); }
    const Terms* get(Path path) const override;
    Terms* replace(Path path, const Terms* by) const override;
    Terms* replace(const Terms* x, const Terms* t) const override;

    std::string print() const override;
};

extern Map forall, exists;
class ForallTerm : public Term
{
public:
    ForallTerm(Variable var, Terms* term)
        : Term(forall, {&var, term}) { Term::boundVar(var); }
    ForallTerm(const ForallTerm& one) = default;
    ~ForallTerm() override = default;

    ForallTerm* clone() const override
    { return (new ForallTerm(*this)); }
    Terms* replace(const Terms* x, const Terms* t) const override;
    std::string print() const override { return printQ(); }
};

class ExistsTerm : public Term
{
public:
    ExistsTerm(Variable var, Terms* term)
        : Term(exists, {&var, term}) { boundVar(var); }
    ExistsTerm(const ExistsTerm& one) = default;
    ~ExistsTerm() override = default;

    ExistsTerm* clone() const override
    { return (new ExistsTerm(*this)); }
    Terms* replace(const Terms* x, const Terms* t) const override;
    std::string print() const override { return printQ(); }
};

#endif //TEST_BUILD_LOGIC_HPP
