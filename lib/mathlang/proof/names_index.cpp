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

void NameSpaceIndex::add(const NamesType& name, Definition* where) {
    if (isThatName(name))
        throw name_doubling(Parser2::texLexer.print(name));
    data[name] = where;
}

bool NameSpaceIndex::isThatName(const NamesType& name) const
{ return (data.find(name) != data.end()); }

std::vector<NameSpaceIndex::NamesType> NameSpaceIndex::getNames() const {
    std::vector<NamesType> buf;
    for (auto& n : data)
            buf.push_back(n.first);
    return buf;
}

std::set<std::string> NameSpaceIndex::getNamesStr() const {
    std::set<std::string> buf;
    for (auto& n : data)
        buf.insert(Parser2::texLexer.print(n.first));
    return buf;
}

Definition* NameSpaceIndex::get(const NamesType& name) const {
    if (isThatName(name))
        return data.at(name);
    throw no_name(Parser2::texLexer.print(name));
}
