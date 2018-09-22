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

void NameSpaceIndex::add(NameSpaceIndex::NameTy type,
                         const std::string& name, Definition* where) {
    if (isSomeType(name)) {
        if (data.at(name).first == type)
            data.at(name).second.insert(where);
        else
            throw name_doubling(name);
    }
    else
        data[name] = std::make_pair(type, std::set<Definition*>({where}));
}

bool NameSpaceIndex::isThatType(const std::string& name, const NameTy& type) const {
    auto search = data.find(name);
    return (search != data.end() && search->second.first == type);
}
bool NameSpaceIndex::isSomeType(const std::string& name) const
{ return (data.find(name) != data.end()); }

std::set<std::string> NameSpaceIndex::getNames(NameTy type) const {
    std::set<std::string> buf;
    for (auto& n : data)
        if (n.second.first == type)
            buf.insert(n.first);
    return buf;
}

std::set<Definition*> NameSpaceIndex::get(NameTy type, const std::string& name) const {
    if (isThatType(name, type))
        return data.at(name).second;
    throw no_name(name);
}
