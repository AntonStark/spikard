//
// Created by anton on 10.01.17.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <string>
#include <set>
#include <stdexcept>
#include <vector>
#include <sstream>
#include "../../json.hpp"

#include "logic.hpp"

class AbstrDef;
class NameSpaceIndex
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
    MathType getT(const std::string& name) const;
    Variable getV(const std::string& name) const;
    Symbol   getS(const std::string& name) const;
};
typedef NameSpaceIndex::NameTy NameTy;

class Hierarchy;
using json = nlohmann::json;

class Node;
class Hierarchy
/// Этот класс обеспечивает древовидную структуру. Ни больше ни меньше.
{
private:
    Node* _parent;
protected:
    Node* getParent() const { return _parent; }
//    virtual Hierarchy* getByPass(Path path) = 0;
    Hierarchy(Node* parent) : _parent(parent) {}
public:
    Hierarchy() = default;
    virtual ~Hierarchy() = default;
    Hierarchy(const Hierarchy&) = delete;
    Hierarchy& operator=(const Hierarchy&) = delete;

    /*const Terms* getTerms(Path pathToTerm)
    {
        Hierarchy* termItem = getByPass(pathToTerm);
        if (auto t = dynamic_cast<Statement*>(termItem))
            return t->get();
        else if (auto v = dynamic_cast<DefVar*>(termItem))
            return v;
        else
            return nullptr;
    }*/
    size_t getNumber() const;
    /*
    // У следующей функции такой странный дизайн, потому что хочется отдавать
    // MlObj для каждой конструкции новым сообщением (вызов write(InfoType, string)
    // А здесь до функционала плагина не добравться, в то же время нельзя пускать
    // плагин до внутренного устройства класса.
    void printMlObjIncr(std::list<std::string>& toOut) const
    {
        if (newInfo.first)
        {
            auto e = subs.end();
            for (auto it = newInfo.second; it != e; ++it)
                toOut.push_back((*it)->toMlObj().dump());
        }
        newInfo.first = false;
    }*/

    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    virtual std::string toString() const;
    virtual json toJson() const;
    virtual json toMlObj() const = 0;
};

class PrimaryNode;
class Item : public Hierarchy
/// Этот класс для первичных конструкций как определения и утверждения
/// Родителем может быть только BunchNode, содержащая первичное
{
protected:
    explicit Item(Node* parent) : Hierarchy(parent) {}
public:
};

class Naming
/// Этот класс обеспечивает интерфейс работы с именами
{
public:
    virtual const NameSpaceIndex& index() const = 0;
    friend class AbstrDef;
    virtual void registerName(
            NameTy type, const std::string& name, AbstrDef* where) = 0;
};

class ListStorage
{
private:
    typedef std::list<Hierarchy*> HiList;
    HiList subs;
    mutable std::pair<bool, HiList::const_iterator> newInfo;
protected:
    ListStorage() : newInfo(false, subs.begin()) {}
    void push(Hierarchy* item)
    {
        subs.push_back(item);
        if (!newInfo.first)
            newInfo = {true, std::prev(subs.end())};
    }
    void resetInfoFlag() { newInfo = {true, subs.begin()}; }
public:
    virtual ~ListStorage()
    {
        for (auto& s : subs)    // Таким образом элемент владеет своими subs, поэтому
            delete s;           // они должны создаваться в куче
    }

    size_t checkChildsNumber(const Hierarchy* child) const
    {
        size_t n = 1;
        for (const auto& s : subs)
            if (s == child)
                return n;
            else
                ++n;
        return 0;
    }
};
class Node : public Hierarchy, public Naming, public ListStorage
/// Этот класс представляет группирующие объекты
// Родителем может быть не только BranchNode, но и
// PrimaryNode (создание теоремы в лекции)
{
protected:
    explicit Node(Node* parent) : Hierarchy(parent)
    {
        if (parent)
            getParent()->push(this);
    }
public:
    Node(): Hierarchy(nullptr) {}
    ~Node() override = default;

protected:
    /*Hierarchy* getByPass(Path path) override
    {
        // теперь используются относительные пути
        Hierarchy* target = this;
        while (!path.empty())
        {
            if (path.front() > target->subs.size())
                return nullptr;
            if (path.front() < 1)
                target = getParent();
            else
                target = *std::next(target->subs.begin(), path.front()-1);
            path.pop_front();
        }
        return target;
    }*/
};
size_t Hierarchy::getNumber() const
{
    auto parent = getParent();
    if (!parent)
        return 0;
    return getParent()->checkChildsNumber(this);
}

class Closure : public virtual Node
/// Этот класс описывает узлы с внутренним хранением имён как в Теоремах и Курсах
{
private:
    NameSpaceIndex atTheEnd;    // Здесь хранится NSI, соответствующее концу Closure,
                                // потому что запись ведётся именно в конец.
                                // Вставки Def-ов влекут обновление.

    explicit Closure(Node* parent) : Node(parent)
    {
        auto _parent = getParent();
        if (_parent)
            atTheEnd = _parent->index();
    }
public:
    Closure() : Closure(nullptr) {}
    const NameSpaceIndex& index() const override { return atTheEnd; }
    void registerName(
            NameTy type, const std::string &name, AbstrDef* where) override
    { atTheEnd.add(type, name, where); }
};
class Appending : public virtual Node
/// Этот класс описывает узлы с внешним хранением имён как в Лекциях и Разделах
{
private:
    friend class Section;
    explicit Appending(Node* parent)
            : Node(parent) {}
public:
    const NameSpaceIndex& index() const override
    { return getParent()->index(); }

    void registerName(
            NameTy type, const std::string& name, AbstrDef* where) override
    { getParent()->registerName(type, name, where); }
};

class PrimaryNode : public virtual Node
/// Этот класс служит для группировки первичных понятий
{
protected:
    explicit PrimaryNode(Node* parent): Node(parent) {}
public:
    ~PrimaryNode() override = default;
};
class BranchNode : public virtual Node
/// Это класс для группировки групп
{
protected:
    explicit BranchNode(BranchNode* parent): Node(parent) {}
public:
    BranchNode() : Node() {}
    ~BranchNode() override = default;
};

class Course : public BranchNode, public Closure
{
private:
    std::string _title;
public:
    explicit Course(std::string title = "")
            : BranchNode(), Closure(), _title(std::move(title)) {}
    ~Course() override = default;
};

class Section : public BranchNode, public Appending
{
private:
    size_t _n;
    std::string _title;
    explicit Section(BranchNode* parent, std::string title = "")
            : BranchNode(parent), Appending(parent),
              _title(std::move(title)), _n(getParent()->getNumber()) {}
public:
    ~Section() override = default;
};


class AbstrDef;
class Axiom;
class Lecture : public PrimaryNode
/// Этот класс симулирует блок рассуждения и инкапсулирует работу с Namespace.
{
private:
    std::string title;
    NameSpaceIndex atTheEnd;    // Здесь хранится NSI, соответсвующее концу Lecture,
                                // потому что запись ведётся именно в конец.
                                // Вставки Def-ов влекут обновление.
    friend class Axiom;
    Lecture(PrimaryNode* parent, const std::string& _title = "");

    friend class AbstrDef;
    void registerName(NameTy type, const std::string& name, AbstrDef* where);
public:
    ~Lecture() override = default;
    Lecture(const Lecture&) = delete;
    Lecture& operator=(const Lecture&) = delete;

    Lecture(const std::string& _title = "");
    const NameSpaceIndex& index() const { return atTheEnd; }
    //  Таким образом есть два варианта организации размещения
    //  с соблюдением владения со стороны старшего в иерархии:
    //  1) DefType::create(closure, "Logical");
    //  2) closure->defType("Logical");
    //  Первый способ выглядит более громоздко, а второй перегружает
    //  интерфейс Lecture посторонним функционалом, но пусть так
    void startSection(const std::string& title = "");
    Lecture* getSub(const std::string& pToSub);
    void defType(const std::string& typeName);
    void defVar (const std::string& varName, const std::string& typeName);
    void defSym (const std::string& symName,
                 const std::list<std::string>& argT, const std::string& retT);
    void addAxiom(const std::string& axiom);
    void doMP   (const std::string& pPremise, const std::string& pImpl);
    void doSpec (const std::string& pToSpec, const std::string& pToVar);
    void doGen  (const std::string& pToGen,  const std::string& pToVar);

    std::string getTitle() const { return title; }
    void printB(std::ostream& out) const;

    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);
    static Hierarchy* fromJsonE(const json& j)
    { return fromJson(j.at("ItemData")); }

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class AbstrDef : public Item
// Это базовый класс определений, отвечает за регистрацию (тип, имя) в Namespace.
{
public:
    ~AbstrDef() override = default;
    AbstrDef(const AbstrDef&) = delete;
    AbstrDef& operator=(const AbstrDef&) = delete;

    AbstrDef(Lecture* closure, NameTy type, const std::string& name)
            : Item(closure) { closure->registerName(type, name, this); }
};

class DefType : public AbstrDef, public MathType
{
private:
    friend class Lecture;
    DefType(Lecture* closure, const std::string& typeName)
            : AbstrDef(closure, NameTy::MT, typeName), MathType(typeName) {}
public:
    ~DefType() override = default;
    DefType(const DefType&) = delete;
    DefType& operator=(const DefType&) = delete;

    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class DefVar : public AbstrDef, public Variable
{
private:
    friend class Lecture;
    DefVar(Lecture* closure, const std::string& varName, MathType mathType)
            : AbstrDef(closure, NameTy::VAR, varName), Variable(varName, mathType) {}
public:
    ~DefVar() override = default;
    DefVar(const DefVar&) = delete;
    DefVar& operator=(const DefVar&) = delete;

    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class DefSym : public AbstrDef, public Symbol
{
private:
    friend class Lecture;
    DefSym(Lecture* closure, const std::string& symName,
           const std::list<MathType>& argT, MathType retT)
            : AbstrDef(closure, NameTy::SYM, symName), Symbol(symName, argT, retT) {}
public:
    ~DefSym() override = default;
    DefSym(const DefSym&) = delete;
    DefSym& operator=(const DefSym&) = delete;

    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class Statement
{
public:
    virtual const Terms* get() const = 0;
};

extern Term* parse(Axiom* where, std::string source);
class Axiom : public Lecture, public Statement
// Этот класс представляет аксиомы. Наследование от Lecture из-за
// необходиомости хранить переменные при кванторах
{
private:
    const Terms* data;

    friend class Lecture;
    friend class Lexer;
    Axiom(Lecture* closure, std::string source);
public:
    ~Axiom() override = default;
    Axiom(const Axiom&) = delete;
    Axiom& operator=(const Axiom&) = delete;

    const Terms* get() const override { return data; }
    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

class AbstrInf : public Item, public Statement
// Этот класс представлет абстрактное следствие.
{
public:
    enum class InfTy {MP, GEN, SPEC};
    class bad_inf;
private:
    std::vector<Path> premises;
    InfTy type;
public:
    ~AbstrInf() override = default;
    AbstrInf(const AbstrInf&) = delete;
    AbstrInf& operator=(const AbstrInf&) = delete;

    AbstrInf(Lecture* closure, InfTy _type, Path pArg1, Path pArg2)
            : Item(closure), premises({pArg1, pArg2}), type(_type) {}

    std::string getTypeAsStr() const;

    std::string toString() const override;
    json toJson() const override;
    json toMlObj() const override;
};

Terms* modusPonens(const Terms* premise, const Terms* impl);
class InfMP : public AbstrInf
{
private:
    Terms* data;
    friend class Lecture;
    InfMP(Lecture* closure, Path pArg1, Path pArg2);
public:
    ~InfMP() override = default;
    InfMP(const InfMP&) = delete;
    InfMP& operator=(const InfMP&) = delete;

    const Terms* get() const override { return data; }

    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);
};

Terms* specialization(const Terms* general, const Terms* t);
class InfSpec : public AbstrInf
{
private:
    Terms* data;
    friend class Lecture;
    InfSpec(Lecture* closure, Path pArg1, Path pArg2);
public:
    ~InfSpec() override = default;
    InfSpec(const InfSpec&) = delete;
    InfSpec& operator=(const InfSpec&) = delete;

    const Terms* get() const override { return data; }

    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);
};

Term*   generalization  (const Terms* toGen, const Terms* x);
class InfGen : public AbstrInf
{
private:
    Terms* data;
    friend class Lecture;
    InfGen(Lecture* closure, Path pArg1, Path pArg2);
public:
    ~InfGen() override = default;
    InfGen(const InfGen&) = delete;
    InfGen& operator=(const InfGen&) = delete;

    const Terms* get() const override { return data; }

    static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);
};

Path mkPath(std::string source);
#endif //TEST_BUILD_SIGNATURE_HPP
