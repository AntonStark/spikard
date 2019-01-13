//
// Created by anton on 19.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMES_STRATEGIES_HPP
#define SPIKARD_MATHLANG_NAMES_STRATEGIES_HPP

#include "names_index.hpp"
#include "structure.hpp"
#include "../parser/lexeme.hpp"

class Hidden : public NameStoringStrategy
/// Стратегия внутреннего хранения имён как в Теоремах и Курсах
{
private:
    /*  Здесь хранится NSI, соответствующее концу Node,
        потому что запись ведётся именно в конец.
        Вставки Def-ов влекут обновление. */
    NameSpaceIndex atTheEnd;
public:
    Hidden() : Hidden(nullptr) {}
    explicit Hidden(Node* parent) {
        if (parent)
            atTheEnd = parent->index();
    }

    const NameSpaceIndex& index() const override { return atTheEnd; }
    void registerNamed(const NamedTerm* term, Definition* where) override
    { atTheEnd.add(term, where); }
    std::string printType() const override { return "Hidden"; }
};

class Appending : public NameStoringStrategy
/// Стратегия внешнего хранения имён как в Лекциях и Разделах
{
private:
    /*  Здесь хранится NSI, соответствующее концу Node,
    потому что запись ведётся именно в конец.
    Вставки Def-ов влекут обновление. */
    NameSpaceIndex atTheEnd;
    Node* _parent;
public:
    explicit Appending(Node* parent) : _parent(parent) {
        if (parent)
            atTheEnd = parent->index();
    }

    const NameSpaceIndex& index() const override { return atTheEnd; }
    void registerNamed(const NamedTerm* term, Definition* where) override {
        _parent->registerNamedTerm(term, where);
        atTheEnd.add(term, where);
    }
    std::string printType() const override { return "Appending"; }
};

NameStoringStrategy* nssFromStr(std::string str, Node* parent);

#endif //SPIKARD_MATHLANG_NAMES_STRATEGIES_HPP
