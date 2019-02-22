//
// Created by anton on 19.11.17.
//

#ifndef TEST_BUILD_STRUCTURE_HPP
#define TEST_BUILD_STRUCTURE_HPP

#include <list>
#include <set>
#include <stack>

#include "../../../json.hpp"

#include "../basics/tex_name.hpp"

#include "names_index.hpp"

using json = nlohmann::json;

class Representation;
class Node;
class Hierarchy
/// Обеспечивает древовидную структуру.
{
private:
    Node* _parent;
protected:
    explicit Hierarchy(Node* parent);
public:
    Hierarchy() : Hierarchy(nullptr) {};
    virtual ~Hierarchy();
    Hierarchy(const Hierarchy&) = delete;
    Hierarchy& operator=(const Hierarchy&) = delete;

    typedef std::stack<size_t> Path;
    Node* getParent() const { return _parent; }
    virtual Hierarchy* getByPass(Path path) = 0;
    size_t getNumber() const;

    virtual std::string print(Representation* r, bool incremental = true) const = 0;
};

class Item : public Hierarchy
/// Этот класс для первичных конструкций как определения и утверждения
{
protected:
    explicit Item(Node* parent) : Hierarchy(parent) {}

public:
    Hierarchy* getByPass(Path path) override
    { return (path.empty() ? this : nullptr); }
};

class ListStorage
/// Реализация хранения потомков узла
{
public:
    typedef std::list<Hierarchy*> HiList;
    typedef HiList::const_iterator HiList_cIter;
private:
    HiList subs;
    mutable std::pair<bool, HiList_cIter> newInfo;
protected:
    ListStorage() : subs(), newInfo(false, subs.begin()) {}
    void push(Hierarchy* item);
    void forget(Hierarchy* what);
    Hierarchy* getByNumber(size_t number) const;
public:
    virtual ~ListStorage();

    size_t getChNumber(const Hierarchy* child) const;
    std::string print(Representation* r, bool incremental) const;
    HiList_cIter start() const { return newInfo.second; }
    HiList_cIter end() const { return  subs.end(); }
    Hierarchy::Path backLabel() const { return Hierarchy::Path({subs.size()}); }
};

class Node : public Hierarchy, public ListStorage
/// Этот класс представляет группирующие структуры
{
private:
    NameStoringStrategy* _naming;
protected:
    explicit Node(Node* parent, NameStoringStrategy* naming)
            : Hierarchy(parent), _naming(naming) { }
public:
    ~Node() override { delete _naming; }
    using ListStorage::push;
    using ListStorage::forget;
    Hierarchy* getByPass(Path path) override;

    virtual const Indices& index() const
    { return _naming->index(); }
    virtual NameStoringStrategy::PrioritySet getPriority() const
    { return _naming->getPriority(); }

    virtual void registerNamed(const NamedEntity* named, const MathType* type, Definition* where)
    { _naming->registerNamed(named, type, where); }
    virtual void prioritize(const NamedEntity* name1, const NamedEntity* name2)
    { _naming->prioritize(name1, name2); }
    NameSpaceIndex::NamesSameType getNames(const MathType* type) const
    { return _naming->getNames(type); }

    Definition* get(const std::string& name);
    virtual std::string print(Representation* r, bool incremental = true) const override
    { return ListStorage::print(r, incremental); }
    std::string nssType() const { return _naming->printType(); }
};


class NamedNode;
class DefType;
class DefAtom;
class DefFunct;
class DefConnective;
class TermsBox;
class Inference;

class Representation
{
public:
    virtual ~Representation() = default;
    virtual std::string str() = 0;
    // далее следуют методы построения некоторого
    // представления для всех тех классов, для которых это
    // имеет смысл (т.е. для конкретных классов иерархии)
    virtual void process(const ListStorage*) = 0;
    virtual void process(const NamedNode*) = 0;
    virtual void process(const DefType*) = 0;
    virtual void process(const DefAtom*) = 0;
    virtual void process(const DefFunct*) = 0;
    virtual void process(const DefConnective*) = 0;
    virtual void process(const TermsBox*) = 0;
    virtual void process(const Inference*) = 0;
};

Hierarchy::Path mkPath(std::string source);
std::string pathToStr(Hierarchy::Path path);

#endif //TEST_BUILD_STRUCTURE_HPP
