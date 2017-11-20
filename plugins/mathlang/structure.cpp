//
// Created by anton on 19.11.17.
//

#include "structure.hpp"

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
                         const std::string& name, AbstrDef* where)
{
    if (!isSomeType(name))
        data.insert({name, {type, where}});
    else
        throw name_doubling(name);
}

bool NameSpaceIndex::isThatType(const std::string& name, const NameTy& type) const
{
    auto search = data.find(name);
    return (search != data.end() && search->second.first == type);
}
bool NameSpaceIndex::isSomeType(const std::string& name) const
{ return (data.find(name) != data.end()); }

std::set<std::string> NameSpaceIndex::getNames(NameTy type) const
{
    std::set<std::string> buf;
    for (auto& n : data)
        if (n.second.first == type)
            buf.insert(n.first);
    return buf;
}

AbstrDef* NameSpaceIndex::get(NameTy type, const std::string& name) const {
    if (isThatType(name, type))
        return data.at(name).second;
    throw no_name(name);
}

size_t Hierarchy::getNumber() const
{
    auto parent = getParent();
    if (!parent)
        return 0;
    return getParent()->checkChildsNumber(this);
}

Path mkPath(std::string source)
{
    // string  ->  list<size_t>
    // (1.2.2) }-> {1,2,2}
    Path target;
    std::map<char, unsigned> digits = {{'0', 0}, {'1', 1},
                                       {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5},
                                       {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}};
    if (source.front() == '(')
    {
        source.pop_back();
        source.erase(source.begin());
    }
    size_t buf = 0;
    for (int i = 0; i < source.length(); ++i)
    {
        auto search = digits.find(source[i]);
        if (search != digits.end())
        {
            buf *= 10;
            buf += search->second;
        }
        else if (source[i] == '.')
        {
            target.push_back(buf);
            buf = 0;
        }
        else
            throw std::invalid_argument("Путь \"" + source + "\" некорректен.");
    }
    target.push_back(buf);
    return target;
}
std::string pathToStr(Path path)
{
    std::stringstream ss; ss << "(";
    if (!path.empty())
    {
        ss << path.front();
        auto e = path.end();
        for (auto it = std::next(path.begin()); it != e; ++it)
            ss << "." << *it;
    }
    ss << ")";
    return ss.str();
}
