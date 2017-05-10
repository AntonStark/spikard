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
        out << '[' << i << "]:\n";
        subs[i]->print(out);
    }
}