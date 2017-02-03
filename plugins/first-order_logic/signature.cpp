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
    names[NameTy::PRED] = {}; names[NameTy::FUNC] = {};
    names[NameTy::CONS] = {}; names[NameTy::VARS] = {};
}

bool Namespace::isThatType(const std::string& name, const NameTy& type) const
{ return (names.at(type).find(name) != names.at(type).end()); }
bool Namespace::isSomeSym(const std::string& name) const
{ return (isThatType(name, NameTy::PRED) || isThatType(name, NameTy::FUNC) ||
          isThatType(name, NameTy::CONS) || isThatType(name, NameTy::VARS)); }

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


void TermsFactory::addC(const std::string& name)
{ C.add(name); }
void TermsFactory::addV(const std::string& name)
{ V.add(name); }

Constant* TermsFactory::getC(const std::string& name) const
{ return C.get(name); }
Variable* TermsFactory::getV(const std::string& name) const
{ return V.get(name); }

Term* TermsFactory::makeTerm(Function* f, std::list<Terms*> args)
{ return T.make({f, args}); }


FormulasFactory::FormulasFactory()
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
        : names(), R(names), F(names), terms(names), formulas()
{
    for (auto r : _R)
        addP(r.first, r.second);
    for (auto f : _F)
        addF(f.first, f.second);
    for (auto c : _C)
        addC(c);
}

bool Signature::isPred(const std::string& name) const
{ return (names.isThatType(name, NameTy::PRED)); }
bool Signature::isFunc(const std::string& name) const
{ return (names.isThatType(name, NameTy::FUNC)); }
bool Signature::isCons(const std::string& name) const
{ return (terms.isCons(name)); }
bool Signature::isVar(const std::string& name) const
{ return (terms.isVar(name)); }

void Signature::addP(const std::string& name, unsigned arity)
{ R.add(name, arity); }
void Signature::addF(const std::string& name, unsigned arity)
{ F.add(name, arity); }
void Signature::addC(const std::string& name)
{ terms.addC(name); }
void Signature::addV(const std::string& name)
{ terms.addV(name); }

Predicate* Signature::getP(const std::string& name) const
{ return R.get(name); }
Function* Signature::getF(const std::string& name) const
{ return F.get(name); }
Constant* Signature::getC(const std::string& name) const
{ return terms.getC(name); }
Variable* Signature::getV(const std::string& name) const
{ return terms.getV(name); }

unsigned Signature::arity(const std::string& name) const
{
    if (isPred(name))
    { return getP(name)->getArity(); }
    else if (isFunc(name))
    { return getF(name)->getArity(); }
    else
    { return static_cast<unsigned>(-1);}
}
