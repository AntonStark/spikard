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

std::set<std::string> NameSpaceIndex::getNames(NameTy type) const
{
    std::set<std::string> buf;
    for (auto& n : names)
        if (n.second == type)
            buf.insert(n.first);
    return buf;
}

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
        atTheEnd = parent->atTheEnd;
}
Section::Section(const std::string& _title)
        : HierarchyItem(), title(_title) {}

void Section::registerName(NameTy type, const std::string& name, AbstrDef* where)
{ atTheEnd.add(type, name, where); }

void Section::pushSection(const std::string& title)
{ new Section(this, title); }
void Section::pushDefType(const std::string& typeName)
{ new DefType(this, typeName); }
void Section::pushDefVar(const std::string& varName, const std::string& typeName)
{ new DefVar(this, varName, index().getT(typeName)); }
void Section::pushDefSym(const std::string& symName, const std::list<std::string>& argT, const std::string& retT)
{
    std::list<MathType> argMT;
    for (auto& a : argT)
        argMT.push_back(index().getT(a));
    new DefSym(this, symName, argMT, index().getT(retT));
}
void Section::pushAxiom(const std::string& axiom)
{ new Axiom(this, axiom); }

Axiom::Axiom(Section* closure, std::string source)
        : Section(closure), Term(parse(this, source))
{
    if (getType() != logical_mt)
        throw std::invalid_argument("Аксиома должна быть логического типа.\n");
}
