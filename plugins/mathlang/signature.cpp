//
// Created by anton on 10.01.17.
//

#include "signature.hpp"

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

void NameSpaceIndex::add(NameSpaceIndex::NameTy type, const std::string& name, AbstrDef* where)
{
    if (!isSomeType(name))
    {
        names.insert({name, type});
        index.insert({name, where});
    }
    else
        throw name_doubling(name);
}

bool NameSpaceIndex::isThatType(const std::string& name, const NameTy& type) const
{
    auto search = names.find(name);
    return (search != names.end() && search->second == type);
}
bool NameSpaceIndex::isSomeType(const std::string& name) const
{ return (names.find(name) != names.end()); }

MathType NameSpaceIndex::getT(const std::string& name) const
{
    if (isThatType(name, NameTy::MT))
        return *dynamic_cast<DefType*>(index.at(name));
    throw no_name(name);
}
Variable NameSpaceIndex::getV(const std::string& name) const
{
    if (isThatType(name, NameTy::VAR))
        return *dynamic_cast<DefVar*>(index.at(name));
    throw no_name(name);
}
Symbol NameSpaceIndex::getS(const std::string& name) const
{
    if (isThatType(name, NameTy::SYM))
        return *dynamic_cast<DefSym*>(index.at(name));
    throw no_name(name);
}

class Namespace::name_doubling : public std::invalid_argument
{
public:
    name_doubling(const std::string& symName)
            : std::invalid_argument("Попытка дублирования имени \"" + symName +"\".\n") {}
};
class Namespace::no_name : public std::invalid_argument
{
public:
    no_name(const std::string& symName)
            : std::invalid_argument("Имя \"" + symName + "\" не определено.\n") {}
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
        throw no_name(name);
}
void Namespace::checkSym(const std::string& name) const
{
    if (!isSomeSym(name))
        throw no_name(name);
}

void Namespace::addSym(const std::string& name, const NameTy& type)
{
    if (isSomeSym(name))
        throw name_doubling(name);
    else
        names.at(type).insert(name);
}
void Namespace::delSym(const std::string& name, const NameTy& type)
{
    if (isThatType(name, type))
        names.at(type).erase(name);
    else
        throw no_name(name);
}


Reasoning::~Reasoning()
{
    for (auto& r : subs)
        delete r;
}

Reasoning* Reasoning::get(Path path)
{
    Reasoning* r = this;
    while (path.size() != 0)
    {
        if (path.front()-1 >= r->subs.size())
            return nullptr;
        r = r->subs.at(path.front()-1);
        path.pop_front();
    }
    return r;
}
const Terms* Reasoning::getTerms(Path path) const
{
    const Reasoning* r = this;
    while (path.size() != 0)
    {
        if (path.front()-1 >= r->subs.size())
            return nullptr;
        r = r->subs.at(path.front()-1);
        path.pop_front();
    }

    if (auto sPremise = dynamic_cast<const Statement*>(r))
        return sPremise->get();
    else
        return nullptr;
}

Reasoning* Reasoning::startSub()
{
    subs.push_back(new Reasoning(this));
    return subs.back();
}
Statement* Reasoning::addSub(Terms* monom)
{
    Statement* st = new Statement(this, monom);
    subs.push_back(st);
    return st;
}


Terms* Reasoning::doMP(const Terms* premise, const Terms* impl) const
{
    if (const Term* tI = dynamic_cast<const Term*>(impl))
    {
        Symbol standardImpl("\\Rightarrow ", {2, logical_mt}, logical_mt);
        if (!(tI->Symbol::operator==)(standardImpl))
            return nullptr; //терм impl не является импликацией

        if (!tI->arg(1)->doCompare(premise))
            return nullptr; //посылка импликации impl не совпадает с premise
        return tI->arg(2)->clone();
    }
    else
        return nullptr; //impl не является термом
}
Terms* Reasoning::doSpec(const Terms* general, const Terms* t) const
{
    if (const ForallTerm* fT = dynamic_cast<const ForallTerm*>(general))
    {
        if (fT->arg(1)->getType() == t->getType())
            return fT->arg(2)->replace(fT->arg(1), t);
    }
    else
        return nullptr;
}

bool Reasoning::deduceMP(Path rpPremise, Path rpImpl)
{
    const Terms* premise = getTerms(rpPremise);
    const Terms* impl = getTerms(rpImpl);
    if (!premise || !impl)
        return false; //ошибочный rpPremise и/или rpImpl

    if (Terms* mp = doMP(premise, impl))
    {
        Reasoning* st = new Statement(this, mp, {rpPremise, rpImpl});
        subs.push_back(st);
        return true;
    }
    else
        return false;
}
bool Reasoning::deduceSpec(Path rpGeneral, Path rpT)
{
    const Terms* general = getTerms(rpGeneral);
    const Terms* t = getTerms(rpT);
    if (!general || !t)
        return false; //ошибочный rpGeneral и/или pT

    if (Terms* spec = doSpec(general, t))
    {
        Reasoning* st = new Statement(this, spec, {rpGeneral});
        subs.push_back(st);
        return true;
    }
    else
        return false;
}
bool Reasoning::deduceSpec(Path rpGeneral, Path subTermPath, Path rpT)
{
    const Terms* general = getTerms(rpGeneral);
    const Terms* t = getTerms(rpT);
    if (!general || !t)
        return false; //ошибочный rpGeneral и/или pT
    const Terms* concreteGen = general->get(subTermPath);
    if (!concreteGen)
        return false; //ошибочно задан подтерм-квантор

    if (Terms* spec = doSpec(concreteGen, t))
    {
        Reasoning* st = new Statement(this, general->replace(subTermPath, spec), {rpGeneral});
        subs.push_back(st);
        return true;
    }
    else
        return false;
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

void Reasoning::printNamespace(std::ostream& out) const
{
    out << "SYMS" << std::endl;
    out << "------------" << std::endl;
    for (const auto& s : syms)
        out << s.first << std::endl;
    out << "------------" << std::endl;

    out << "VARS" << std::endl;
    out << "------------" << std::endl;
    for (const auto& v : vars)
        out << v.first << std::endl;
    out << "------------" << std::endl;

    out << "TYPES" << std::endl;
    out << "------------" << std::endl;
    for (const auto& t : types)
        out << t.first << std::endl;
    out << "------------" << std::endl;
}

void Statement::print(std::ostream& out) const
{
    if (comment.length() != 0)
        out << comment << std::endl;

    if (const Variable* v = dynamic_cast<const Variable*>(monom))
        v->print(out);
    else if (const Term* t = dynamic_cast<const Term*>(monom))
        t->print(out);
    /*else if (const QuantedTerm* qt = dynamic_cast<const QuantedTerm*>(monom))
        qt->print(out);*/
    else
        return;
    out << std::endl;
}

HierarchyItem::HierarchyItem(Section* _parent)
        : parent(_parent) { parent->push(this); }
HierarchyItem::~HierarchyItem()
{
    for (auto& s : subs)    // Таким образом элемент владеет своими subs, поэтому
        delete s;           // они должны создаваться в куче
}

Section::Section(Section* _parent, const std::string& _title)
        : HierarchyItem(_parent), title(_title)
{
    auto parent = getParent();
    if (parent)
        index = parent->index;
}
Section::Section(const std::string& _title)
        : HierarchyItem(), title(_title) {}
void Section::registerName(NameSpaceIndex::NameTy type, const std::string& name, AbstrDef* where)
{ index.add(type, name, where); }
MathType Section::getType(const std::string& typeName)
{ return index.getT(typeName); }
void Section::pushDefType(std::string typeName)
{ new DefType(this, typeName); }
void Section::pushDefVar(std::string varName, std::string typeName)
{ new DefVar(this, varName, getType(typeName)); }
void Section::pushDefSym(std::string symName, std::list<std::string> argT, std::string retT)
{
    std::list<MathType> argMT;
    for (auto& a : argT)
        argMT.push_back(getType(a));
    new DefSym(this, symName, argMT, getType(retT));
}

AbstrDef::AbstrDef(Section* closure, NameSpaceIndex::NameTy type, const std::string& name)
        : HierarchyItem(closure)
{ closure->registerName(type, name, this); }

DefType::DefType(Section* closure, const std::string& typeName)
        : AbstrDef(closure, NameSpaceIndex::NameTy::MT, typeName), MathType(typeName) {}
DefVar::DefVar(Section* closure, const std::string& varName, MathType mathType)
        : AbstrDef(closure, NameSpaceIndex::NameTy::VAR, varName), Variable(varName, mathType) {}
DefSym::DefSym(Section* closure, const std::string& symName, std::list<MathType> argT, MathType retT)
        : AbstrDef(closure, NameSpaceIndex::NameTy::SYM, symName), Symbol(symName, argT, retT) {}
