//
// Created by anton on 19.09.18.
//

#include "names_index.hpp"

class NameSpaceIndex::name_doubling : public std::invalid_argument
{
public:
    name_doubling(const std::string& symName)
        : std::invalid_argument("Попытка дублирования имени \"" + symName +"\".\n") {}
};
class NameSpaceIndex::no_name : public std::invalid_argument
{
public:
    no_name(const std::string& symName)
        : std::invalid_argument("Имя \"" + symName + "\" не определено.\n") {}
};

bool NameSpaceIndex::exists(const std::string& str) const
{ return (name2ID.find(str) != name2ID.end()); }

void NameSpaceIndex::add(const NamedEntity* named, const MathType* type, Definition* where) {
    const auto& name = named->getName();
    const auto& str = name->toStr();
    const auto& typeName = type->getName();
    if (!exists(str)) {
        size_t id = definitions.size();
        definitions.push_back(where);
        name2ID[str] = id;

        size_t storageIndex;
        auto searchStorage = type2Storage.find(typeName);
        if (searchStorage == type2Storage.end()) {
            size_t newStorID = names.size();
            names.emplace_back();
            type2Storage[typeName] = newStorID;
            storageIndex = newStorID;
        } else
            storageIndex = searchStorage->second;
        names[storageIndex].push_back(name);
    } else
        throw name_doubling(str);
}

NameSpaceIndex::NamesSameType NameSpaceIndex::getNames(const MathType* type) const {
    const auto& typeName = type->getName();
    auto search = type2Storage.find(typeName);
    if (search != type2Storage.end())
        return names[search->second];
    else
        return {};
}

std::vector<std::string> NameSpaceIndex::getNamesStr(const MathType* type) const {
    const auto& namesAbs = getNames(type);

    size_t nCount = namesAbs.size();
    std::vector<std::string> namesStr(nCount);
    for (size_t i = 0; i < nCount; ++i)
        namesStr[i] = namesAbs.at(i)->toStr();
    return namesStr;
}

Definition* NameSpaceIndex::get(const AbstractName* name) const {
    const auto& str = name->toStr();
    if (exists(str))
        return definitions[name2ID.at(str)];
    throw no_name(name->toStr());
}

std::vector<const AbstractName*>::const_iterator
find(
    const std::vector<const AbstractName*>& vec,
    const AbstractName* name)
{
    for (auto it = vec.begin(); it != vec.end(); ++it)
        if (**it == *name)
            return it;
    return vec.end();
}

NameSpaceIndex::NamesSameType NameStoringStrategy::getNames(const MathType* type) const {
    auto indices = index();
    auto priority = getPriority();

    auto conn = indices.connectives.getNames(type);
    /// утверждения о приоритетах применяем в порядке их ввода
    /// применяем = если с2 > c1 с2 переставляется непосредственно перед c1 и всё
    for (const auto& p : priority) {      /// (name1, name2) \in priority означает name1 < name2
        const AbstractName* name1 = p.first;
        const AbstractName* name2 = p.second;
        auto it1 = find(conn, name1);
        auto it2 = find(conn, name2);
        /// требуется, чтобы приоритет name2 был выше name1 т.е. it2 должен быть меньше it1
        if (it1 != conn.end() && it2 != conn.end() && it1 < it2) {  // нужна перестановка *it2 перед *it1
            conn.erase(it2);
            conn.insert(it1, name2);
        }
    }
    auto names = indices.names.getNames(type);
    conn.insert(conn.end(), names.begin(), names.end());
    return conn;
}
