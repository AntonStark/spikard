//
// Created by anton on 04.01.17.
//

#include "signature.hpp"

class Signature::sym_sets_intersection : public std::invalid_argument
{
public:
    sym_sets_intersection(const std::string& symName)
            : std::invalid_argument("Пересечение множеств символов в конструкторе Signature по символу " + symName +".\n") {}
};
class Signature::sym_exists : public std::invalid_argument
{
public:
    sym_exists(const std::string& symName)
            : std::invalid_argument("Символ с именем \"" + symName + "\" уже определён.\n") {}
};

Signature::Signature(std::list<std::pair<std::string, unsigned> > _R,
                     std::list<std::pair<std::string, unsigned> > _F,
                     std::list<std::string> _C)
        : R(_R.begin(), _R.end()), F(_F.begin(), _F.end()), C(_C.begin(), _C.end())
{
    for (auto f : F)
        if (R.find(f.first) != R.end())
            throw sym_sets_intersection(f.first);
    for (auto c : C)
        if (R.find(c) != R.end())
            throw sym_sets_intersection(c);
    for (auto c : C)
        if (F.find(c) != F.end())
            throw sym_sets_intersection(c);
}

Signature::nameT Signature::checkName(const std::string& name) const
{
    if (isPredName(name))
        return nameT::predicate;
    if (isFuncName(name))
        return nameT::function;
    if (isConsName(name))
        return nameT::constant;
    return nameT::none;
}

unsigned Signature::arity(const std::string& name) const
{
    if (isPredName(name))
    { return R.at(name); }
    else if (isFuncName(name))
    { return F.at(name); }
    else
    { return static_cast<unsigned>(-1);}
}

void Signature::addPred(const std::string& name, unsigned arity)
{
    if (checkName(name) != nameT::none)
        throw sym_exists(name);
    R.insert(std::make_pair(name, arity));
}
void Signature::addFunc(const std::string& name, unsigned arity)
{
    if (checkName(name) != nameT::none)
        throw sym_exists(name);
    F.insert(std::make_pair(name, arity));
}
void Signature::addConst(const std::string& name)
{
    if (checkName(name) != nameT::none)
        throw sym_exists(name);
    C.insert(name);
}

std::shared_ptr<Predicate> Signature::getPred(const std::string& name) const
{
    if (isPredName(name))
        return (std::make_shared<Predicate>(name, arity(name)) );
    else
        throw std::invalid_argument("Предикат \"" + name + "\" не определён.\n");
}
std::shared_ptr<Function> Signature::getFunc(const std::string& name) const
{
    if (isFuncName(name))
        return (std::make_shared<Function>(name, arity(name)) );
    else
        throw std::invalid_argument("Функиональный символ \"" + name + "\" не определён.\n");
}
std::shared_ptr<Constant> Signature::getCons(const std::string& name) const
{
    if (isConsName(name))
        return (std::make_shared<Constant>(name) );
    else
        throw std::invalid_argument("Константа \"" + name + "\" не определена.\n");
}

unsigned long Signature::maxLength(nameT type) const
{
    unsigned long maxLen = 0;
    switch (type)
    {
        case nameT::predicate:
        {
            for (auto r : R)
                if (r.first.length() > maxLen)
                    maxLen = r.first.length();
            break;
        }
        case nameT::function:
        {
            for (auto f : F)
                if (f.first.length() > maxLen)
                    maxLen = f.first.length();
            break;
        }
        case nameT::constant:
        {
            for (auto c : C)
                if (c.length() > maxLen)
                    maxLen = c.length();
            break;
        }
        case nameT::none: {}
    }
    return maxLen;
}