//
// Created by anton on 04.01.17.
//

#include "signature.hpp"

class Signature::sym_doubling : public std::invalid_argument
{
public:
    sym_doubling(const std::string& symName)
            : std::invalid_argument("Попытка дублирования символа \"" + symName +"\".\n") {}
};
/*class Signature::sym_exists : public std::invalid_argument
{
public:
    sym_exists(const std::string& symName)
            : std::invalid_argument("Символ с именем \"" + symName + "\" уже определён.\n") {}
};*/
class Signature::no_sym : public std::invalid_argument
{
public:
    no_sym(const std::string& symName)
            : std::invalid_argument("Cимвол \"" + symName + "\" не определён.\n") {}
};

Signature::Signature(std::list<std::pair<std::string, unsigned> > _R,
                     std::list<std::pair<std::string, unsigned> > _F,
                     std::list<std::string> _C)
{
    /*//Проверка, что в списках нет повторяющихся имён
    std::map<std::string, unsigned> tR;
    std::map<std::string, unsigned> tF;
    std::set<std::string> tC;
    for (auto r : _R)
    {
        if (tR.find(r.first) != tR.end())
            throw sym_doubling(r.first);
        else
            tR.insert(r);
    }
    for (auto f : _F)
    {
        if (tF.find(f.first) != tF.end())
            throw sym_doubling(f.first);
        else
            tF.insert(f);
    }
    for (auto c : _C)
    {
        if (tC.find(c) != tC.end())
            throw sym_doubling(c);
        else
            tC.insert(c);
    }

    //Првоерка, что задаваемые множества не будут пересекаться по именам
    for (auto f : tF)
        if (tR.find(f.first) != tR.end())
            throw sym_doubling(f.first);
    for (auto c : tC)
        if (tR.find(c) != tR.end())
            throw sym_doubling(c);
    for (auto c : tC)
        if (tF.find(c) != tF.end())
            throw sym_doubling(c);

    //Инициализация множеств
    for (auto r : tR)
        R[r.first] = (new Predicate(r.first, r.second) );
    for (auto f : tF)
        F[f.first] = (new Function(f.first, f.second) );
    for (auto c : tC)
        C[c] = (new Constant(c) );*/

    for (auto r : _R)
        addP(r.first, r.second);
    for (auto f : _F)
        addF(f.first, f.second);
    for (auto c : _C)
        addC(c);
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
    { return R.at(name)->getArity(); }
    else if (isFuncName(name))
    { return F.at(name)->getArity(); }
    else
    { return static_cast<unsigned>(-1);}
}

void Signature::addP(const std::string& name, unsigned arity)
{
    if (checkName(name) != nameT::none)
        throw sym_doubling(name);
    R[name] = std::shared_ptr<Predicate>(new Predicate(name, arity) );
}
void Signature::addF(const std::string& name, unsigned arity)
{
    if (checkName(name) != nameT::none)
        throw sym_doubling(name);
    F[name] = std::shared_ptr<Function>(new Function(name, arity) );
}
void Signature::addC(const std::string& name)
{
    if (checkName(name) != nameT::none)
        throw sym_doubling(name);
    C[name] = std::shared_ptr<Constant>(new Constant(name) );
}

std::shared_ptr<Predicate> Signature::getP(const std::string& name) const
{
    if (isPredName(name))
        return R.at(name);
    else
        throw no_sym(name);
}
std::shared_ptr<Function> Signature::getF(const std::string& name) const
{
    if (isFuncName(name))
        return F.at(name);
    else
        throw no_sym(name);
}
std::shared_ptr<Constant> Signature::getC(const std::string& name) const
{
    if (isConsName(name))
        return C.at(name);
    else
        throw no_sym(name);
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
                if (c.first.length() > maxLen)
                    maxLen = c.first.length();
            break;
        }
        case nameT::none: {}
    }
    return maxLen;
}

