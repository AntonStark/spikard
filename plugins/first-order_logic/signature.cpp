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

Namespace::Namespace()
{
    names[NameTy::PRED] = {};
    names[NameTy::FUNC] = {};
    names[NameTy::VARS] = {};
}

bool Namespace::isThatType(const std::string& name, const NameTy& type) const
{ return (names.at(type).find(name) != names.at(type).end()); }
bool Namespace::isSomeSym(const std::string& name) const
{ return (isThatType(name, NameTy::PRED) ||
          isThatType(name, NameTy::FUNC) ||
          isThatType(name, NameTy::VARS)); }

void Namespace::checkSym(const std::string& name, const NameTy& type) const
{
    if (!isThatType(name, type))
        throw no_sym(name);
}
void Namespace::checkSym(const std::string& name) const
{
    if (!isSomeSym(name))
        throw no_sym(name);
}

void Namespace::addSym(const std::string& name, const NameTy& type)
{
    if (isSomeSym(name))
        throw sym_doubling(name);
    else
        names.at(type).insert(name);
}
void Namespace::delSym(const std::string& name, const NameTy& type)
{
    if (isThatType(name, type))
        names.at(type).erase(name);
    else
        throw no_sym(name);
}

bool TermsFactory::isVar(const std::string& name) const
{ return V.is(name); }
void TermsFactory::addV(const std::string& name)
{ V.add(name); }
Variable* TermsFactory::getV(const std::string& name) const
{ return V.get(name); }
Variable* TermsFactory::makeVar(const std::string& name)
{
    if (!V.is(name))
        V.add(name);
    return V.get(name);
}

Term* TermsFactory::makeTerm(Function* f, std::list<Terms*> args)
{ return T.make({f, args}); }


FormulasFactory::FormulasFactory()
        : M(), A(), F()
{
    makeMod(MType::NOT);
    makeMod(MType::AND);
    makeMod(MType::OR);
    makeMod(MType::THAN);
}

Modifier* FormulasFactory::makeMod(MType _type, Variable* _arg)
{ return M.make({_arg, _type}); }
Formula* FormulasFactory::makeFormula(Predicate* p, std::list<Terms*> args)
{ return A.make({p, args}); }
Formula* FormulasFactory::makeFormula(Modifier* _mod, Formula* F1, Formula* F2)
{ return F.make({_mod, {F1, F2}}); }


Signature::Signature(std::list<std::pair<std::string, unsigned> > _R,
                     std::list<std::pair<std::string, unsigned> > _F,
                     std::list<std::string> _C)
        : names(), R(names), F(names)
{
    for (auto r : _R)
        R.add(r.first, r.second);
    for (auto f : _F)
        F.add(f.first, f.second);
    for (auto c : _C)
        F.add(c, 0);
}

bool Signature::isPred(const std::string& name) const
{ return R.is(name); }
bool Signature::isFunc(const std::string& name) const
{ return F.is(name); }

Predicate* Signature::getP(const std::string& name) const
{ return R.get(name); }
Function* Signature::getF(const std::string& name) const
{ return F.get(name); }

unsigned Signature::arity(const std::string& name) const
{
    if (isPred(name))
    { return getP(name)->getArity(); }
    else if (isFunc(name))
    { return getF(name)->getArity(); }
    else
    { return static_cast<unsigned>(-1);}
}
