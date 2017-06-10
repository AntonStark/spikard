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
    names[NameTy::SYM] = {};
    names[NameTy::VAR] = {};
    names[NameTy::MT] = {};
}

bool Namespace::isThatType(const std::string& name, const NameTy& type) const
{ return (names.at(type).find(name) != names.at(type).end()); }
bool Namespace::isSomeSym(const std::string& name) const
{ return (isThatType(name, NameTy::SYM) ||
          isThatType(name, NameTy::VAR) ||
          isThatType(name, NameTy::MT)); }

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


Reasoning::~Reasoning()
{
    for (auto& r : subs)
        delete r;
}

Reasoning* Reasoning::get(RPath path)
{
    Reasoning* r = this;
    while (path.size() != 0)
    {
        if (path.front()-1 >= r->subs.size())
            return nullptr;
        r = (*r)[path.front()-1];
        path.pop_front();
    }
    return r;
}

Reasoning& Reasoning::startSub()
{
    subs.push_back(new Reasoning(this));
    return *subs.back();
}
void Reasoning::addSub(Terms* monom)
{
    Reasoning* t = new Statement(this, monom);
    subs.push_back(t);
}

bool Reasoning::deduceMP(RPath pPremise, RPath pImpl)
{
    const Terms* premise;
    if (Statement* sPremise = dynamic_cast<Statement*>(get(pPremise)))
        premise = sPremise->get();
    else
        return false; //ошибочный pPremise

    const Terms* impl;
    if (Statement* sImpl = dynamic_cast<Statement*>(get(pImpl)))
        impl = sImpl->get();
    else
        return false; //ошибочный pImpl
    
    if (const Term* tI = dynamic_cast<const Term*>(impl))
    {
        Symbol standardImpl("\\Rightarrow ", {2, logical_mt}, logical_mt);
        if (!(tI->Symbol::operator==)(standardImpl))
            return false;
        const Terms* arg1 = tI->arg(1);
        if (!arg1->doCompare(premise))
            return false; //посылка импликации impl не совпадает с premise

        Reasoning* st = new Statement(this, tI->arg(2), {premise, impl});
        subs.push_back(st);
        return true;
    }
    else
        return false; //терм impl не является импликацией
}

const Reasoning* Reasoning::isNameExist(const std::string& name, const NameTy& type) const
{
    if (names.isThatType(name, type))
        return this;
    else if (parent != nullptr)
        return parent->isNameExist(name, type);
    else
        return nullptr;
}

void Reasoning::addSym(Symbol sym)
{
    std::string name = sym.getName();
    names.addSym(name, NameTy::SYM);
    syms.insert({name, sym});
}
void Reasoning::addSym(std::list<Symbol> syms)
{
    for (auto& s : syms)
        addSym(s);
}
void Reasoning::addSym(const std::string& name,
                       std::list<MathType> argT, MathType retT)
{
    Symbol sym(name, argT, retT);
    names.addSym(name, NameTy::SYM);
    syms.insert({name, sym});
}

void Reasoning::addVar(Variable var)
{
    std::string name = var.getName();
    names.addSym(name, NameTy::VAR);
    vars.insert({name, var});
}
void Reasoning::addVar(const std::string& name, MathType type)
{
    names.addSym(name, NameTy::VAR);
    Variable var(name, type);
    vars.insert({name, var});
}

void Reasoning::addType(MathType type)
{
    std::string name = type.getName();
    names.addSym(name, NameTy::MT);
    types.insert({name, type});
}
void Reasoning::addType(const std::string& name)
{
    names.addSym(name, NameTy::MT);
    MathType type(name);
    types.insert({name, type});
}

Symbol Reasoning::getS(const std::string& name) const
{
    const Reasoning* reas = isNameExist(name, NameTy::SYM);
    if (reas != nullptr)
        return reas->syms.at(name);
    else
        throw std::invalid_argument("No sym.\n");
}
Variable Reasoning::getV(const std::string& name) const
{
    const Reasoning* reas = isNameExist(name, NameTy::VAR);
    if (reas != nullptr)
        return reas->vars.at(name);
    else
        throw std::invalid_argument("No sym.\n");
}
MathType Reasoning::getT(const std::string& name) const
{
    const Reasoning* reas = isNameExist(name, NameTy::MT);
    if (reas != nullptr)
        return reas->types.at(name);
    else
        throw std::invalid_argument("No sym.\n");
}

void Reasoning::viewSetOfNames(std::set<std::string>& set,
                               const NameTy& type) const
{
    names.viewSetOfNames(set, type);
    if (parent != nullptr)
        parent->viewSetOfNames(set, type);
}

void Reasoning::print(std::ostream& out) const
{
    for (size_t i = 0; i < subs.size(); ++i)
    {
        out << '[' << i+1 << "]:\n";
        subs[i]->print(out);
    }
}

void Statement::print(std::ostream& out) const
{
    if (comment.length() != 0)
        out << comment << std::endl;

    if (const Variable* v = dynamic_cast<const Variable*>(monom))
        v->print(out);
    else if (const Term* t = dynamic_cast<const Term*>(monom))
        t->print(out);
    else if (const QuantedTerm* qt = dynamic_cast<const QuantedTerm*>(monom))
        qt->print(out);
    else
        return;
    out << std::endl;
}
