//
// Created by anton on 21.09.18.
//

#ifndef SPIKARD_MATHLANG_STATEMENT_HPP
#define SPIKARD_MATHLANG_STATEMENT_HPP

#include "../basics/contructed_term.hpp"
#include "../basics/mapterm.hpp"
#include "../basics/terms.hpp"

#include "definition.hpp"
#include "names_strategies.hpp"
#include "structure.hpp"

#include "../parser/parser.hpp"

class Statement
{
public:
    virtual const Terms* get() const = 0;
};

class TermsBox : public Node, public Statement
/// Этот класс представляет аксиомы.
/// Наследование от Node из-за необходиомости хранить имена при кванторах
{
private:
    const Terms* data;
    TermsBox(Node* parent, std::string source);

    friend class PrimaryNode;
    static Hierarchy* fromJson(const json& j, Node* parent = nullptr);
public:
    ~TermsBox() override = default;
    TermsBox(const TermsBox&) = delete;
    TermsBox& operator=(const TermsBox&) = delete;

    const Terms* get() const override { return data; }
    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
};

class Inference : public Item, public Statement
// Этот класс представлет любые следствия.
{
public:
    class bad_inf;
    enum class InfTy {MP, SPEC, APPL, EQL, GEN};

    const std::vector<Path> premises;
    const InfTy type;
    const Terms* data;
protected:
    const Terms* getTerms(Path pathToTerm);
    const Terms* inference();
public:
    ~Inference() override = default;
    Inference(const Inference&) = delete;
    Inference& operator=(const Inference&) = delete;

    static Hierarchy* fromJson(const json& j, Node* parent = nullptr);
    Inference(Node* naming, Path pArg1, Path pArg2,  InfTy _type);

    const Terms* get() const override { return data; }
    std::string getTypeAsStr() const;
    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
};

Terms* modusPonens(const Terms* premise, const Terms* impl);
Terms* specialization(const Terms* general, const Terms* t);
Terms* application(const Terms* term, const Terms* theorem);
Terms* equalSubst(const Terms* term, const Terms* equality);
Term* generalization(const Terms* toGen, const Terms* x);

#endif //SPIKARD_MATHLANG_STATEMENT_HPP