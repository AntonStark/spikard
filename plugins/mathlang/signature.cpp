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

void NameSpaceIndex::add(NameSpaceIndex::NameTy type,
                         const std::string& name, AbstrDef* where)
{
    if (!isSomeType(name))
        data.insert({name, {type, where}});
    else
        throw name_doubling(name);
}

bool NameSpaceIndex::isThatType(const std::string& name, const NameTy& type) const
{
    auto search = data.find(name);
    return (search != data.end() && search->second.first == type);
}
bool NameSpaceIndex::isSomeType(const std::string& name) const
{ return (data.find(name) != data.end()); }

std::set<std::string> NameSpaceIndex::getNames(NameTy type) const
{
    std::set<std::string> buf;
    for (auto& n : data)
        if (n.second.first == type)
            buf.insert(n.first);
    return buf;
}

MathType NameSpaceIndex::getT(const std::string& name) const
{
    if (isThatType(name, NameTy::MT))
        return *dynamic_cast<DefType*>(data.at(name).second);
    throw no_name(name);
}
Variable NameSpaceIndex::getV(const std::string& name) const
{
    if (isThatType(name, NameTy::VAR))
        return *dynamic_cast<DefVar*>(data.at(name).second);
    throw no_name(name);
}
Symbol NameSpaceIndex::getS(const std::string& name) const
{
    if (isThatType(name, NameTy::SYM))
        return *dynamic_cast<DefSym*>(data.at(name).second);
    throw no_name(name);
}


HierarchyItem::HierarchyItem(Section* _parent)
        : parent(_parent) { parent->push(this); }
HierarchyItem::~HierarchyItem()
{
    for (auto& s : subs)    // Таким образом элемент владеет своими subs, поэтому
        delete s;           // они должны создаваться в куче
}
HierarchyItem* HierarchyItem::get(Path path)
{
    /*HierarchyItem* root = this;
    while (root->getParent())
        root = root->getParent();*/
    // теперь используются относительные пути
    HierarchyItem* target = this;
    while (path.size() != 0)
    {
        if (path.front() > target->subs.size())
            return nullptr;
        if (path.front() < 1)
            target = getParent();
        else
            target = *std::next(target->subs.begin(), path.front()-1);
        path.pop_front();
    }
    return target;
}
const Terms* HierarchyItem::getTerms(Path pathToTerm)
{
    HierarchyItem* termItem = get(pathToTerm);
    if (auto t = dynamic_cast<Statement*>(termItem))
        return t->get();
    else if (auto v = dynamic_cast<DefVar*>(termItem))
        return v;
    else
        return nullptr;
}
void HierarchyItem::print(std::ostream& out) const
{
    size_t n = 1;
    for (const auto& s : subs)
        out << '(' << n++ << "): " << *s << std::endl;
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

void Section::startSection(const std::string& title)
{ new Section(this, title); }
Section* Section::getSub(const std::string& pToSub)
{
    if (auto s = dynamic_cast<Section*>( get(mkPath(pToSub)) ))
        return s;
    else
        return nullptr;
}
void Section::defType(const std::string& typeName)
{ new DefType(this, typeName); }
void Section::defVar(const std::string& varName, const std::string& typeName)
{ new DefVar(this, varName, index().getT(typeName)); }
void Section::defSym(const std::string& symName,
                     const std::list<std::string>& argT, const std::string& retT)
{
    std::list<MathType> argMT;
    for (auto& a : argT)
        argMT.push_back(index().getT(a));
    new DefSym(this, symName, argMT, index().getT(retT));
}
void Section::addAxiom(const std::string& axiom)
{ new Axiom(this, axiom); }
void Section::doMP(const std::string& pPremise, const std::string& pImpl)
{ new InfMP(this, mkPath(pPremise), mkPath(pImpl)); }
void Section::doSpec(const std::string& pToSpec, const std::string& pToVar)
{ new InfSpec(this, mkPath(pToSpec), mkPath(pToVar)); }
void Section::doGen(const std::string& pToGen, const std::string& pToVar)
{ new InfGen(this, mkPath(pToGen), mkPath(pToVar)); }
void Section::print(std::ostream& out) const
{ out << "Раздел \"" << title << "\"." << std::endl; }
void Section::printB(std::ostream& out) const
{
    print(out);
    HierarchyItem::print(out);
}

void DefType::print(std::ostream& out) const
{ out << "Объявлен тип " << getName() << "."; }
void DefVar::print(std::ostream& out) const
{ out << "Добавлена переменная " << getName() << " типа " << getType().getName() << "."; }
void DefSym::print(std::ostream& out) const
{
    out << "Введен символ " << getName() << " : ";
    auto argTypes = getSign().first;
    if (argTypes.size() > 0)
    {
        out << argTypes.front().getName();
        auto e = argTypes.end();
        for (auto it = next(argTypes.begin()); it != e; ++it)
            out << " x " << it->getName();
    }
    out << " -> " << getType().getName() << ".";
}

Axiom::Axiom(Section* closure, std::string source)
        : Section(closure), data(parse(this, source))
{
    if (data->getType() != logical_mt)
        throw std::invalid_argument("Аксиома должна быть логического типа.\n");
}
void Axiom::print(std::ostream& out) const
{
    out << "Пусть ";
    Statement::print(out);
}

Path mkPath(std::string source)
{
    // string  ->  list<size_t>
    // (1.2.2) }-> {1,2,2}
    Path target;
    std::map<char, unsigned> digits = {{'0', 0}, {'1', 1},
                                  {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5},
                                  {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}};
    if (source.front() == '(')
    {
        source.pop_back();
        source.erase(source.begin());
    }
    unsigned buf = 0;
    for (int i = 0; i < source.length(); ++i)
    {
        auto search = digits.find(source[i]);
        if (search != digits.end())
        {
            buf *= 10;
            buf += search->second;
        }
        else if (source[i] == '.')
        {
            target.push_back(buf);
            buf = 0;
        }
        else
            throw std::invalid_argument("Путь \"" + source + "\" некорректен.");
    }
    target.push_back(buf);
    return target;
}
std::string pathToStr(Path path)
{
    std::stringstream ss; ss << "(";
    if (!path.empty())
    {
        ss << path.front();
        auto e = path.end();
        for (auto it = std::next(path.begin()); it != e; ++it)
            ss << "." << *it;
    }
    ss << ")";
    return ss.str();
}

class AbstrInf::bad_inf : public std::invalid_argument
{
public:
    bad_inf() : std::invalid_argument("Неподходящие аргументы для данного вывода.") {}
};

void AbstrInf::print(std::ostream& out) const
{
    out << "По ";
    switch (type)
    {
        case InfTy::MP :
            out << "MP ";   break;
        case InfTy::SPEC :
            out << "Spec "; break;
        case InfTy::GEN :
            out << "Gen ";  break;
    }
    out << "из " << pathToStr(*premises.begin()) << " и "
    << pathToStr(*std::next(premises.begin())) << " следует: ";
    Statement::print(out);
}

Terms* modusPonens(const Terms* premise, const Terms* impl)
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
InfMP::InfMP(Section* closure, Path pArg1, Path pArg2)
        : AbstrInf(closure, AbstrInf::InfTy::MP, pArg1, pArg2),
          data(modusPonens(getParent()->getTerms(pArg1), getParent()->getTerms(pArg2)))
{ if (!data) throw bad_inf(); }

Terms* specialization(const Terms* general, const Terms* t)
{
    if (const ForallTerm* fT = dynamic_cast<const ForallTerm*>(general))
    {
        if (fT->arg(1)->getType() == t->getType())
            return fT->arg(2)->replace(fT->arg(1), t);
    }
    else
        return nullptr;
}
InfSpec::InfSpec(Section* closure, Path pArg1, Path pArg2)
        : AbstrInf(closure, AbstrInf::InfTy::SPEC, pArg1, pArg2),
          data(specialization(getParent()->getTerms(pArg1), getParent()->getTerms(pArg2)))
{ if (!data) throw bad_inf(); }

Term* generalization  (const Terms* toGen, const Terms* x)
{
    if (const Variable* v = dynamic_cast<const Variable*>(x))
    {
        if (const Term* tG = dynamic_cast<const Term*>(toGen))
        if (tG->free.find(*v) != tG->free.end())
            return new ForallTerm(*v, tG->clone());
    }
    else
        return nullptr;
}
InfGen::InfGen(Section* closure, Path pArg1, Path pArg2)
        : AbstrInf(closure, AbstrInf::InfTy::GEN, pArg1, pArg2),
          data(generalization(getParent()->getTerms(pArg1), getParent()->getTerms(pArg2)))
{ if (!data) throw bad_inf(); }


json HierarchyItem::toJson() const
{
    json temp;
    for (const auto& s : subs)
        temp.push_back(s->toJson());
    return temp;
}
json Section::toJson() const
{
    json temp;
    temp["ItemType"] = "Section";
    temp["ItemData"] = {"title", title};
    temp["ItemSubs"] = HierarchyItem::toJson();
    return temp;
}
json DefType::toJson() const
{
    json temp;
    temp["ItemType"] = "DefType";
    temp["ItemData"] = {"name", getName()};
    return temp;
}
json DefVar::toJson() const
{
    json temp;
    temp["ItemType"] = "DefVar";
    temp["ItemData"] = { {"name", getName()},
                         {"type", getType().getName()} };
    return temp;
}
json DefSym::toJson() const
{
    json temp;
    auto symInfo = getSign();
    std::vector<std::string> argT;
    for (const auto& a : symInfo.first)
        argT.push_back(a.getName());

    temp["ItemType"] = "DefSym";
    temp["ItemData"] = { {"name", getName()},
                         {"argT", argT},
                         {"retT", symInfo.second.getName()}};
    return temp;
}
json Axiom::toJson() const
{
    json temp;
    std::stringstream ss;
    data->print(ss);
    temp["ItemType"] = "Axiom";
    temp["ItemData"] = {"axiom", ss.str()};
    temp["ItemSubs"] = HierarchyItem::toJson();
    return temp;
}

Serializable* HierarchyItem::fromJson(Section *parent, const json &j) { return nullptr; }
Serializable* Section::fromJson(Section *parent, const json &j)
{

    return nullptr;
}
