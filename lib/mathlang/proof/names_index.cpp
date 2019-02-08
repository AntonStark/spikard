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
