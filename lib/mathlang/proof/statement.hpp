//
// Created by anton on 21.09.18.
//

#ifndef SPIKARD_MATHLANG_STATEMENT_HPP
#define SPIKARD_MATHLANG_STATEMENT_HPP

#include "../consepts/terms.hpp"

#include "../parser/parser.hpp"

#include "definition.hpp"
#include "names_strategies.hpp"
#include "structure.hpp"

class Statement
{
public:
    virtual const AbstractTerm* get() const = 0;
};

class TermsBox : public Item, public Statement
/// Этот класс представляет аксиомы.
/// Наследование от Node из-за необходиомости хранить имена при кванторах
{
private:
    const AbstractTerm* data;
    TermsBox(Node* parent, std::string source);

//    static Hierarchy* fromJson(const json& j, Node* parent = nullptr);
public:
    ~TermsBox() override = default;
    TermsBox(const TermsBox&) = delete;
    TermsBox& operator=(const TermsBox&) = delete;

    const AbstractTerm* get() const override { return data; }
    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
    static TermsBox* create(PrimaryNode* parent, std::string source)
    { return new TermsBox(parent, source); }
};

class Inference : public Item, public Statement
// Этот класс представлет любые следствия.
{
public:
    class bad_inf;
    enum class InfTy {MP, SPEC, APPL, EQL, GEN};

    const std::vector<Path> premises;
    const InfTy type;
    const AbstractTerm* data;
protected:
    const AbstractTerm* getTerms(Path pathToTerm);
    const AbstractTerm* inference();
public:
    ~Inference() override = default;
    Inference(const Inference&) = delete;
    Inference& operator=(const Inference&) = delete;

//    static Hierarchy* fromJson(const json& j, Node* parent = nullptr);
    Inference(Node* naming, Path pArg1, Path pArg2,  InfTy _type);

    const AbstractTerm* get() const override { return data; }
    std::string getTypeAsStr() const;
    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
};

//extern Map standardImpl;

/*AbstractTerm* modusPonens(const AbstractTerm* premise, const AbstractTerm* impl);
AbstractTerm* specialization(const AbstractTerm* general, const AbstractTerm* t);
AbstractTerm* application(const AbstractTerm* term, const AbstractTerm* theorem);
AbstractTerm* equalSubst(const AbstractTerm* term, const AbstractTerm* equality);
Term* generalization(const AbstractTerm* toGen, const AbstractTerm* x);*/

#endif //SPIKARD_MATHLANG_STATEMENT_HPP
