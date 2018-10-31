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
                         const NamesType& name, Definition* where) {
    if (isSomeType(name))
        throw name_doubling(Parser2::texLexer.print(name));
    data[name] = std::make_pair(type, where);
}

bool NameSpaceIndex::isThatType(const NamesType& name, const NameTy& type) const {
    auto search = data.find(name);
    return (search != data.end() && search->second.first == type);
}
bool NameSpaceIndex::isSomeType(const NamesType& name) const
{ return (data.find(name) != data.end()); }

std::set<NameSpaceIndex::NamesType> NameSpaceIndex::getNames(NameTy type) const {
    std::set<NamesType> buf;
    for (auto& n : data)
        if (n.second.first == type)
            buf.insert(n.first);
    return buf;
}

Definition* NameSpaceIndex::get(NameTy type, const NamesType& name) const {
    if (isThatType(name, type))
        return data.at(name).second;
    throw no_name(Parser2::texLexer.print(name));
}
