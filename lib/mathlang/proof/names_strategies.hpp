//
// Created by anton on 19.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMES_STRATEGIES_HPP
#define SPIKARD_MATHLANG_NAMES_STRATEGIES_HPP

#include "names_index.hpp"
#include "structure.hpp"
#include "../parser/lexeme.hpp"
#include "../basics/connectives.hpp"

struct IndexCollection
{
    NameSpaceIndex primary;
    NameSpaceIndex connectives;

    void add(const NamedEntity* named, Definition* where) {
        if (auto conn = dynamic_cast<const PrintableConnective*>(named))
            connectives.add(conn, conn->resultType(), where);
        else if (auto var = dynamic_cast<const Variable*>(named))
            primary.add(var, var->getType(), where);
        else if (auto pty = dynamic_cast<const PrimaryType*>(named))
            primary.add(pty, pty->getType(), where);
        else
            throw std::logic_error("конкретный тип named какой-то ещё");
    }
};


class Hidden : public NameStoringStrategy
/// Стратегия внутреннего хранения имён как в Теоремах и Курсах
{
private:
    Indices indices;
    PrioritySet priority;
public:
    Hidden() : Hidden(nullptr) {}
    explicit Hidden(Node* parent) {
        if (parent) {
            indices = parent->index();
            priority = parent->getPriority();
        }
    }

    const Indices& index() const override { return indices; }
    PrioritySet getPriority() const override { return priority; }
    void registerNamed(const NamedEntity* named, const MathType* type, Definition* where) override {
        if (auto conn = dynamic_cast<const PrintableConnective*>(named))
            indices.connectives.add(named, type, where);
        else
            indices.names.add(named, type, where);
    }
    void prioritize(const NamedEntity* name2, const NamedEntity* name1) override
    { priority.insert(std::make_pair(name1->getName(), name2->getName())); }
    std::string printType() const override { return "Hidden"; }
};

class Appending : public NameStoringStrategy
/// Стратегия внешнего хранения имён как в Лекциях и Разделах
{
private:
    Indices indices;
    /// (name1, name2) \in priority означает name1 < name2
    PrioritySet priority;
    Node* _parent;
public:
    explicit Appending(Node* parent) : _parent(parent) {
        indices = parent->index();
        priority = parent->getPriority();
        /// отдельный индекс для связок нужен, потому что связка
        /// автоматически приоритетнее всякого имени
    }

    const Indices& index() const override { return indices; }
    PrioritySet getPriority() const override { return priority; }
    void registerNamed(const NamedEntity* named, const MathType* type, Definition* where) override {
        _parent->registerNamed(named, type, where);

        if (auto conn = dynamic_cast<const PrintableConnective*>(named))
            indices.connectives.add(named, type, where);
        else
            indices.names.add(named, type, where);
    }
    void prioritize(const NamedEntity* name2, const NamedEntity* name1) override {
        _parent->prioritize(name1, name2);
        priority.insert(std::make_pair(name1->getName(), name2->getName()));
    }
    std::string printType() const override { return "Appending"; }
};

typedef NameStoringStrategy::BasicNSSTypes NssType;
NameStoringStrategy* nssFromStr(std::string str, Node* parent);
NameStoringStrategy* nssFromNSSType(NssType type, Node* parent);

#endif //SPIKARD_MATHLANG_NAMES_STRATEGIES_HPP
