//
// Created by anton on 15.12.16.
//

#include "logic.hpp"

bool Named::operator==(const Named& one) const
{ return (name == one.name); }
bool Symbol::operator==(const Symbol& one) const
{ return (this->Named::operator==)(one); }
bool Map::operator==(const Map& one) const
{ return (arity == one.arity); }
bool Predicate::operator==(const Predicate& one) const
{ return ( /*(sigma.get() == one.sigma.get()) && */(this->Symbol::operator==)(one) && (this->Map::operator==)(one) ); }
bool Function::operator==(const Function &one) const
{ return ( (this->Symbol::operator==)(one) && (this->Map::operator==)(one) ); }




std::ostream& operator<< (std::ostream& os, const Printable& pr)
{
    pr.print(os);
    return os;
}
void Symbol::print(std::ostream &out) const
{ out << getName(); }
void ParenSymbol::print(std::ostream &out) const
{
    out << '(';
    if (!args.empty())
    {
        auto lit = args.begin(), le = args.end();
        (*lit)->print(out);
        while (++lit != le)
        {
            out << ", ";
            (*lit)->print(out);
        }
    }
    out << ')';
}
void Term::print(std::ostream &out) const
{
    Function::print(out);
    ParenSymbol::print(out);
}

class ParenSymbol::nArg_arity_error : public std::invalid_argument
{
public:
    nArg_arity_error()
            : std::invalid_argument("Кол-во аргументов не соответствует арности символа.\n") {}
};
void ParenSymbol::argCheck(Map* f, std::list<Terms*> _args)
{
    if (_args.size() != f->getArity())
        throw nArg_arity_error();
}
void ParenSymbol::argCheck(std::shared_ptr<Map> f, TermsList _args)
{
    if (_args.size() != f->getArity())
        throw nArg_arity_error();
}
ParenSymbol::ParenSymbol(std::list<Terms*> _args)
{
    for (auto t : _args)
    {
        std::shared_ptr<Terms> tSh(t->clone());
        if (std::shared_ptr<Variable> vSh = std::dynamic_pointer_cast<Variable>(tSh))
            vars.insert(vSh);
        args.push_back(tSh);
    }
}
ParenSymbol::ParenSymbol(TermsList _args) : args(_args)
{
    for (auto t : _args)
    {
        if (t->isVariable())
            vars.insert(std::dynamic_pointer_cast<Variable>(t) );
    }
}

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
