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


class Node;
class Hierarchy
/// Обеспечивает древовидную структуру.
{
private:
    Node* _parent;
protected:
    Node* getParent() const { return _parent; }
    virtual Hierarchy* getByPass(Path path) = 0;
    explicit Hierarchy(Node* parent) : _parent(parent) {}
public:
    Hierarchy() : Hierarchy(nullptr) {};
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

    /*static Hierarchy* fromJson(const json& j, Lecture* parent = nullptr);

    virtual std::string toString() const;
    virtual json toJson() const;
    virtual json toMlObj() const = 0;*/
};


class Naming
/// Интерфейс работы с именами со стороны узлов
{
public:
    virtual const NameSpaceIndex& index() const = 0;
    friend class AbstrDef;
    virtual void registerName(
            NameTy type, const std::string& name, AbstrDef* where) = 0;
};
class ListStorage
/// Реализация хранения потомков узла
{
public:
    typedef std::list<Hierarchy*> HiList;
private:
    HiList subs;
    mutable std::pair<bool, HiList::const_iterator> newInfo;
protected:
    ListStorage() : subs(), newInfo(false, subs.begin()) {}
    void push(Hierarchy* item)
    {
        subs.push_back(item);
        if (!newInfo.first)
            newInfo = {true, std::prev(subs.end())};
    }
    void resetInfoFlag() { newInfo = {true, subs.begin()}; }
    Hierarchy* getByNumber(size_t number) const
    {
        if (number > subs.size())
            return nullptr;
        else
            return *std::next(subs.begin(), number-1);
    }
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
{
protected:
    explicit Node(Node* parent) : Hierarchy(parent)
    {
        if (parent)
            getParent()->push(this);
    }

    Hierarchy* getByPass(Path path) override {
        // теперь используются относительные пути
        if (!path.empty())
        {
            auto n = path.front();
            path.pop_front();
            if (n < 1)
                return getParent()->getByPass(path);
            else
                return getByNumber(n)->getByPass(path);
        }
        return this;
    }
public:
    Node(): Hierarchy(nullptr) {}
    ~Node() override = default;
};


class Inner : public virtual Node
/// Этот класс описывает узлы с внутренним хранением имён как в Теоремах и Курсах
{
private:
    NameSpaceIndex atTheEnd;    // Здесь хранится NSI, соответствующее концу Inner,
                                // потому что запись ведётся именно в конец.
                                // Вставки Def-ов влекут обновление.
protected:
    explicit Inner(Node* parent) : Node(parent)
    {
        auto _parent = getParent();
        if (_parent)
            atTheEnd = _parent->index();
    }
public:
    Inner() : Inner(nullptr) {}
    const NameSpaceIndex& index() const override { return atTheEnd; }
    void registerName(
            NameTy type, const std::string &name, AbstrDef* where) override
    { atTheEnd.add(type, name, where); }
};
class Appending : public virtual Node
/// Этот класс описывает узлы с внешним хранением имён как в Лекциях и Разделах
{
protected:
    explicit Appending(Node* parent)
            : Node(parent) {}
public:
    const NameSpaceIndex& index() const override
    { return getParent()->index(); }

    void registerName(
            NameTy type, const std::string& name, AbstrDef* where) override
    { getParent()->registerName(type, name, where); }
};

Path mkPath(std::string source);
std::string pathToStr(Path path);

#endif //TEST_BUILD_STRUCTURE_HPP
