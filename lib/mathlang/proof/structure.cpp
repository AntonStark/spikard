//
// Created by anton on 19.11.17.
//

#include "structure.hpp"

Hierarchy::Hierarchy(Node* parent) : _parent(parent) {
    if (parent)
        getParent()->push(this);
}
Hierarchy::~Hierarchy() {
    if (auto *p = getParent())
        p->forget(this);
}
size_t Hierarchy::getNumber() const {
    auto parent = getParent();
    if (!parent)
        return 0;
    return getParent()->getChNumber(this);
}

void ListStorage::push(Hierarchy* item) {
    subs.push_back(item);
    if (!newInfo.first)
        newInfo = {true, std::prev(subs.end())};
}
void ListStorage::forget(Hierarchy* what) {
    auto e = subs.end();
    for (auto it = subs.begin(); it != e; ++it)
        if (*it == what) {
            auto updNInfo = prev(subs.erase(it));
            newInfo = {false, updNInfo};
            break;
        }
}
Hierarchy* ListStorage::getByNumber(size_t number) const {
    if (number > subs.size())
        return nullptr;
    else
        return *std::next(subs.begin(), number-1);
}
ListStorage::~ListStorage() {
    while (!subs.empty())   // Таким образом элемент владеет своими subs, поэтому
        delete subs.back(); // они должны создаваться в куче
}
size_t ListStorage::getChNumber(const Hierarchy* child) const {
    size_t n = 1;
    for (const auto& s : subs)
        if (s == child)
            return n;
        else
            ++n;
    return 0;
}
std::string ListStorage::print(Representation* r, bool incremental) const {
    if (!incremental)
        newInfo = {true, subs.begin()};
    r->process(this);
    newInfo.first = false;
    return r->str();
}

Hierarchy* Node::getByPass(Path path) {
    // теперь используются относительные пути
    if (!path.empty()) {
        auto n = path.top(); path.pop();
        if (n < 1)
            return getParent()->getByPass(path);
        else
            return getByNumber(n)->getByPass(path);
    }
    return this;
}

Definition* Node::get(const std::string& name) {
    auto* texName = new TexName(name); // fixme shared_pointers для имён
    return index().get(texName);
}

Hierarchy::Path mkPath(std::string source)
{
    // string  ->  stack<size_t>
    // (1.2.7) }-> {7,2,1}
    Hierarchy::Path straight;
    std::map<char, unsigned> digits = {{'0', 0}, {'1', 1},
                                       {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5},
                                       {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}};
    if (source.front() == '(') {
        source.pop_back();
        source.erase(source.begin());
    }
    size_t buf = 0;
    for (int i = 0; i < source.length(); ++i) {
        auto search = digits.find(source[i]);
        if (search != digits.end()) {
            buf *= 10;
            buf += search->second;
        }
        else if (source[i] == '.') {
            straight.push(buf);
            buf = 0;
        }
        else
            return Hierarchy::Path();
    }
    straight.push(buf);
    Hierarchy::Path reversed;
    while (!straight.empty()) {
        reversed.push(straight.top());
        straight.pop();
    }
    return reversed;
}
std::string pathToStr(Hierarchy::Path path)
{
    std::stringstream ss;
    ss << "(";
    if (!path.empty()) {
        ss << path.top();
        path.pop();
        while (!path.empty()) {
            ss << "." << path.top();
            path.pop();
        }
    }
    ss << ")";
    return ss.str();
}
