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
    void registerNamed(const NamedEntity* named, const MathType* type, Definition* where) override
    { atTheEnd.add(named, type, where); }
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
    void registerNamed(const NamedEntity* named, const MathType* type, Definition* where) override {
        _parent->registerNamed(named, type, where);
        atTheEnd.add(named, type, where);
    }
    std::string printType() const override { return "Appending"; }
};

typedef NameStoringStrategy::BasicNSSTypes NssType;
NameStoringStrategy* nssFromStr(std::string str, Node* parent);
NameStoringStrategy* nssFromNSSType(NssType type, Node* parent);

#endif //SPIKARD_MATHLANG_NAMES_STRATEGIES_HPP
