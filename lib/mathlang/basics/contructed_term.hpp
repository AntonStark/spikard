//
// Created by anton on 28.09.18.
//

#ifndef SPIKARD_CONTRUCTEDTERM_HPP
#define SPIKARD_CONTRUCTEDTERM_HPP

#include "terms.hpp"
#include "mapterm.hpp"
#include "named_term.hpp"

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

class Term : public Terms, public ParenSymbol // fixme зачем наследование от ParenSymbol? разве они в отношении "является"?!
{
protected:
    void boundVar(Variable var);
    std::string printQ() const;
public:
    typedef std::set<Variable> VarSet;
    enum class QType {FORALL, EXISTS};
    static std::map<QType, const AbstractName*> qword;

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
    ForallTerm(Variable* var, Terms* term)
        : Term(forall, {var, term}) { Term::boundVar(*var); }
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
    ExistsTerm(Variable* var, Terms* term)
        : Term(exists, {var, term}) { boundVar(*var); }
    ExistsTerm(const ExistsTerm& one) = default;
    ~ExistsTerm() override = default;

    ExistsTerm* clone() const override
    { return (new ExistsTerm(*this)); }
    Terms* replace(const Terms* x, const Terms* t) const override;
    std::string print() const override { return printQ(); }
};

#endif //SPIKARD_CONTRUCTEDTERM_HPP
