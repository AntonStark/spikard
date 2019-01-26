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
    const auto& str = named->getName()->toStr();
    const auto& typeName = type->getName();
    if (!exists(str)) {
        size_t id = names.size();
        names.push_back(named->getName());
        definitions.push_back(where);

        auto searchTypeIDs = type2IDs.find(typeName);
        if (searchTypeIDs == type2IDs.end())
            type2IDs[typeName] = {id};
        else
            searchTypeIDs->second.push_back(id);
        name2ID[str] = id;
    } else
        throw name_doubling(str);
}

std::vector<const AbstractName*> NameSpaceIndex::getNames(const MathType* type) const {
    if (type == nullptr)
        return names;

    std::vector<const AbstractName*> buf;
    auto search = type2IDs.find(type->getName());
    if (search != type2IDs.end()) {
        const std::vector<size_t>& thatTypeIDs = search->second;
        for (const auto& id : thatTypeIDs)
            buf.push_back(names[id]);
    }
    return buf;
}

std::vector<std::string> NameSpaceIndex::getNamesStr(const MathType* type) const {
    const auto& names = getNames(type);
    std::vector<std::string> buf;
    for (auto& name : names)
        buf.push_back(name->toStr());
    return buf;
}

Definition* NameSpaceIndex::get(const AbstractName* name) const {
    const auto& str = name->toStr();
    if (exists(str))
        return definitions[name2ID.at(str)];
    throw no_name(name->toStr());
}
