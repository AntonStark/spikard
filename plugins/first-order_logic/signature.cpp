//
// Created by anton on 10.01.17.
//

#include "signature.hpp"

class Namespace::sym_doubling : public std::invalid_argument
{
public:
    sym_doubling(const std::string& symName)
            : std::invalid_argument("Попытка дублирования символа \"" + symName +"\".\n") {}
};
class Namespace::no_sym : public std::invalid_argument
{
public:
    no_sym(const std::string& symName)
            : std::invalid_argument("Cимвол \"" + symName + "\" не определён.\n") {}
};

bool Namespace::isPred(const std::string& name) const
{ return (predicates.find(name) != predicates.end()); }
bool Namespace::isFunc(const std::string& name) const
{ return (functions.find(name) != functions.end()); }
bool Namespace::isCons(const std::string& name) const
{ return (constants.find(name) != constants.end()); }
bool Namespace::isVar(const std::string& name) const
{ return (variables.find(name) != variables.end()); }
bool Namespace::isSomeSym(const std::string& name) const
{ return (isPred(name) || isFunc(name) ||
          isCons(name) || isVar(name)); }

void Namespace::checkPred(const std::string& name) const
{
    if (!isPred(name))
        throw no_sym(name);
}
void Namespace::checkFunc(const std::string& name) const
{
    if (!isFunc(name))
        throw no_sym(name);
}
void Namespace::checkCons(const std::string& name) const
{
    if (!isCons(name))
        throw no_sym(name);
}
void Namespace::checkVar(const std::string& name) const
{
    if (!isVar(name))
        throw no_sym(name);
}
void Namespace::checkSym(const std::string& name) const
{
    if (!isSomeSym(name))
        throw no_sym(name);
}

void Namespace::addPred(const std::string& name)
{
    if (isSomeSym(name))
        throw sym_doubling(name);
    else
        predicates.insert(name);
}
void Namespace::addFunc(const std::string& name)
{
    if (isSomeSym(name))
        throw sym_doubling(name);
    else
        functions.insert(name);
}
void Namespace::addCons(const std::string& name)
{
    if (isSomeSym(name))
        throw sym_doubling(name);
    else
        constants.insert(name);
}
void Namespace::addVar(const std::string& name)
{
    if (!isSomeSym(name))
        throw sym_doubling(name);
    else
        variables.insert(name);
}


Signature::Signature(std::list<std::pair<std::string, unsigned> > _R,
                     std::list<std::pair<std::string, unsigned> > _F,
                     std::list<std::string> _C)
        : names(), termsStorage(names)
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

Signature::~Signature()
{
    for (auto r : R)
        delete r.second;
    for (auto f : F)
        delete f.second;
    for (auto c : C)
        delete c.second;
}

Signature::nameT Signature::checkName(const std::string& name) const
{
    if (isPred(name))
        return nameT::predicate;
    if (isFunc(name))
        return nameT::function;
    if (isCons(name))
        return nameT::constant;
    return nameT::none;
}

unsigned Signature::arity(const std::string& name) const
{
    if (isPred(name))
    { return R.at(name)->getArity(); }
    else if (isFunc(name))
    { return F.at(name)->getArity(); }
    else
    { return static_cast<unsigned>(-1);}
}

void Signature::addP(const std::string& name, unsigned arity)
{
    names.addPred(name);
    R[name] = new Predicate(name, arity);
}
void Signature::addF(const std::string& name, unsigned arity)
{
    names.addFunc(name);
    F[name] = new Function(name, arity);
}
void Signature::addC(const std::string& name)
{
    termsStorage.addC(name);
}

Predicate* Signature::getP(const std::string& name) const
{
    names.checkPred(name);
    //если бы символ не был добавлен, схватили бы исключение от Namespace
    return R.at(name);
}
Function* Signature::getF(const std::string& name) const
{
    names.checkFunc(name);
    return F.at(name);
}
Constant* Signature::getC(const std::string& name) const
{
    return termsStorage.getC(name);
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

TermsFactory::~TermsFactory()
{
    for (auto t : T)
        delete t.second;
    for (auto v : V)
        delete v.second;
    for (auto c : C)
        delete c.second;
}

void TermsFactory::addC(const std::string& name)
{
    names.addCons(name);
    C[name] = new Constant(name);
}
void TermsFactory::addV(const std::string& name)
{
    names.addVar(name);
    V[name] = new Variable(name);
}

Constant* TermsFactory::getC(const std::string& name) const
{
    names.checkCons(name);
    //если бы символ не был добавлен, схватили бы исключение от Namespace
    return C.at(name);
}
Variable* TermsFactory::getV(const std::string& name) const
{
    names.checkVar(name);
    return V.at(name);
}

Term* TermsFactory::makeTerm(Function* f, std::list<Terms*> args)
{
    //TODO ещё не тестирована
    auto pair = std::make_pair(f, args);
    Term* t;
    auto search = T.find(pair);
    if (search != T.end())
        t = search->second;
    else
        T[pair] = t = new Term(f, args);
    return t;
    /*try {t = T.at(pair);}
    catch (std::out_of_range)
    { t = T[pair] = new Term(f, args);}
    return t;*/
}