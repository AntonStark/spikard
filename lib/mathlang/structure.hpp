//
// Created by anton on 19.11.17.
//

#ifndef TEST_BUILD_STRUCTURE_HPP
#define TEST_BUILD_STRUCTURE_HPP

#include "../../json.hpp"

#include "logic.hpp"

using json = nlohmann::json;

class AbstrDef;
class NameSpaceIndex
/// Инкапсулирует работу с именами: имя-сущность, уникальность, доступ
{
public:
    enum class NameTy {SYM, VAR, MT};
private:
    std::map<std::string, std::pair<NameTy, AbstrDef*> > data;

    class name_doubling;
    class no_name;
public:
    void add(NameTy type, const std::string& name, AbstrDef* where);
    bool isThatType(const std::string& name, const NameTy& type) const;
    bool isSomeType(const std::string& name) const;

    std::set<std::string> getNames(NameTy type) const;
    AbstrDef* get(NameTy type, const std::string& name) const;
};
typedef NameSpaceIndex::NameTy NameTy;

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

    Node* getParent() const { return _parent; }
    virtual Hierarchy* getByPass(Path path) = 0;
    size_t getNumber() const;

    virtual std::string print(Representation* r, bool incremental = true) const = 0;
};


class NameStoringStrategy
/// Интерфейс работы с именами со стороны узлов
{
public:
    virtual const NameSpaceIndex& index() const = 0;
    friend class AbstrDef;
    virtual void registerName(
            NameTy type, const std::string& name, AbstrDef* where) = 0;

    virtual std::string printType() const = 0;
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

    virtual const NameSpaceIndex& index() const
    { return _naming->index(); }
    virtual void registerName(NameTy type, const std::string& name, AbstrDef* where)
    { _naming->registerName(type, name, where); }

    virtual std::string print(Representation* r, bool incremental = true) const override
    { return ListStorage::print(r, incremental); }
    std::string nssType() const { return _naming->printType(); }
};


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
    void registerName(
            NameTy type, const std::string &name, AbstrDef* where) override
    { atTheEnd.add(type, name, where); }
    std::string printType() const override { return "Hidden"; }
};

class Appending : public NameStoringStrategy
/// Стратегия внешнего хранения имён как в Лекциях и Разделах
{
private:
    Node* _parent;
public:
    explicit Appending(Node* parent) : _parent(parent) {}

    const NameSpaceIndex& index() const override { return _parent->index(); }
    void registerName(
            NameTy type, const std::string& name, AbstrDef* where) override
    { _parent->registerName(type, name, where); }
    std::string printType() const override { return "Appending"; }
};

NameStoringStrategy* nssFromStr(std::string str, Node* parent);


class NamedNode;
class DefType;
class DefVar;
class DefSym;
class Axiom;
class AbstrInf;

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
    virtual void process(const DefVar*) = 0;
    virtual void process(const DefSym*) = 0;
    virtual void process(const Axiom*) = 0;
    virtual void process(const AbstrInf*) = 0;
};

Path mkPath(std::string source);
std::string pathToStr(Path path);

#endif //TEST_BUILD_STRUCTURE_HPP
