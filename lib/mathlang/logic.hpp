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

class Named
{
private:
    std::string _name;
public:
    Named(std::string name) : _name(std::move(name)) {}
    Named(const Named&) = default;
    virtual ~Named() = default;

    bool operator== (const Named& one) const;
    bool operator< (const Named& other) const;

    std::string getName() const { return _name; }
    void setName(std::string name) { _name = std::move(name); }
};

class MathType
{
public:
    virtual ~MathType() {};
    virtual bool isPrimary() const = 0;
    virtual bool operator== (const MathType& other) const = 0;
    bool operator!= (const MathType& other) const
    { return !(*this == other); }
    virtual bool operator< (const MathType& other) const = 0;

    virtual std::string getName() const = 0;
};

class PrimaryMT : public MathType
{
private:
    std::string _type;
public:
    PrimaryMT(std::string type) : _type(std::move(type)) {}
    PrimaryMT(const PrimaryMT&) = default;
    ~PrimaryMT() override = default;

    bool operator== (const MathType& other) const override;
    bool operator<(const MathType& other) const;
    bool isPrimary() const override { return true; }
    std::string getName() const override;
};
extern PrimaryMT any_mt;
extern PrimaryMT logical_mt;

class ComplexMT : public MathType
{
public:
    typedef std::vector<const MathType*> MTVector;
    enum class SORT {PRODUCT, MAP};
private:
    SORT _sort;
    MTVector _subTypes;
public:
    ComplexMT(MTVector subTypes) : _sort(SORT::PRODUCT), _subTypes(subTypes) {};
    ComplexMT(MTVector args, MTVector rets) : _sort(SORT::MAP) {
        ComplexMT* argMT = new ComplexMT(args);
        ComplexMT* retMT = new ComplexMT(rets);
        _subTypes = {argMT, retMT};
    }
    ComplexMT(const ComplexMT&) = default;
    ~ComplexMT() {
        if (_sort == SORT::MAP) {
            delete _subTypes[0];
            delete _subTypes[1];
        }
    }

    bool operator==(const MathType& one) const override;
    bool operator<(const MathType& other) const override;
    bool isPrimary() const override { return false; }
    std::string getName() const override;
};

typedef std::stack<size_t> Path;
class Terms
{
public:
    virtual const MathType* getType() const = 0;
    virtual bool comp(const Terms* other) const = 0;

    virtual Terms* clone() const = 0;
    virtual const Terms* get(Path path) const = 0;
    virtual Terms* replace(Path path, const Terms* by) const = 0;
    virtual Terms* replace(const Terms* x, const Terms* t) const = 0;

    virtual std::string print() const = 0;
};

class Variable : public Terms, public Named
{
private:
    const MathType* _type;
public:
    Variable(std::string name, const MathType* type)
            : Named(std::move(name)), _type(type) {}
    Variable(const Variable& one) = default;
    ~Variable() override = default;

    const MathType* getType() const override { return _type; }
    bool comp(const Terms* other) const override;

    Variable* clone() const override { return new Variable(*this); }
    const Terms* get(Path path) const override;
    Terms* replace(Path path, const Terms* by) const override;
    Terms* replace(const Terms* x, const Terms* t) const override;

    std::string print() const override;
};

// Указание типов в отображнеии вовлекает семантику, но позволяет более полное описание сущности
// Важно, что с априорной информацией о типах упрощается парсер.
// Это реализация неоднородного символа.
// Подходит для символов малой арности, но не каких-нубудь R^n->R^m
// Отображения сами являются термами, поскольку есть отображения отображений.
// Например, символ взятия производной.
// todo Понятие производного типа. Ведь Тип отображения это новый Тип, производный тип, а не просто его Сигнатура
class Map : public Terms
{
public:
    typedef std::vector<const MathType*> MTVector;
    typedef std::pair<MTVector, const MathType*> Signature;
private:
    const MTVector _argT;
    const MathType* _retT;
    ComplexMT _type;
public:
    Map(MTVector argT, const MathType* retT)
        : _argT(std::move(argT)), _retT(retT), _type(argT, {retT}) {}
    Map(const Map&) = default;
    virtual ~Map() = default;

    bool operator== (const Map& one) const;
    bool operator< (const Map& other) const;

    size_t   getArity() const { return _argT.size(); }
    const MathType* getType() const { return _retT; }
    Signature getSign() const { return {_argT, _retT}; }
    bool matchArgType(const MTVector& otherArgT) const
    { return (otherArgT == _argT); }
};
class Symbol : public Named, public Map
{
public:
    Symbol(std::string name, MTVector argT, const MathType* retT)
        : Named(std::move(name)), Map(std::move(argT), retT) {}
    Symbol(const Symbol&) = default;
    ~Symbol() override = default;

    bool operator== (const Symbol& one) const;
    bool operator<(const Symbol& other) const;

    std::string print() const { return getName(); }
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
    const TermsVector& replace(Path path, const Terms* by) const;
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

    Symbol _f;
    VarSet free;
    Term(Symbol f, TermsVector _args);
    Term(std::set<Symbol> symSet, TermsVector args);
    Term(const Term& one) = default;
    ~Term() override = default;

    size_t getArity() const { return  _f.getArity(); }
    const MathType* getType() const override { return _f.getType(); }
    Symbol getSym() const { return _f; }
    bool comp(const Terms* other) const override;

    Term* clone() const override { return new Term(*this); }
    const Terms* get(Path path) const override;
    Terms* replace(Path path, const Terms* by) const override;
    Terms* replace(const Terms* x, const Terms* t) const override;

    std::string print() const override;
};

extern Symbol forall, exists;
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
