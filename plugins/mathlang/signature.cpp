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


Lecture::Lecture(PrimaryNode* parent, const std::string& _title)
        : PrimaryNode(parent), title(_title)
{
    auto _parent = getParent();
    if (_parent)
        atTheEnd = _parent->atTheEnd;
}
Lecture::Lecture(const std::string& _title)
        : PrimaryNode(), title(_title) {}

void Lecture::registerName(NameTy type, const std::string& name, AbstrDef* where)
{ atTheEnd.add(type, name, where); }

void Lecture::startSection(const std::string& title)
{ new Lecture(this, title); }
Lecture* Lecture::getSub(const std::string& pToSub)
{
    if (auto s = dynamic_cast<Lecture*>( getByPass(mkPath(pToSub)) ))
    {
        s->resetInfoFlag();
        return s;
    }
    else
        return nullptr;
}
void Lecture::defType(const std::string& typeName)
{ new DefType(this, typeName); }
void Lecture::defVar(const std::string& varName, const std::string& typeName)
{ new DefVar(this, varName, index().getT(typeName)); }
void Lecture::defSym(const std::string& symName,
                     const std::list<std::string>& argT, const std::string& retT)
{
    std::list<MathType> argMT;
    for (auto& a : argT)
        argMT.push_back(index().getT(a));
    new DefSym(this, symName, argMT, index().getT(retT));
}
void Lecture::addAxiom(const std::string& axiom)
{ new Axiom(this, axiom); }
void Lecture::doMP(const std::string& pPremise, const std::string& pImpl)
{ new InfMP(this, mkPath(pPremise), mkPath(pImpl)); }
void Lecture::doSpec(const std::string& pToSpec, const std::string& pToVar)
{ new InfSpec(this, mkPath(pToSpec), mkPath(pToVar)); }
void Lecture::doGen(const std::string& pToGen, const std::string& pToVar)
{ new InfGen(this, mkPath(pToGen), mkPath(pToVar)); }

void Lecture::printB(std::ostream& out) const
{
    toString();
    Hierarchy::toString();
}


Axiom::Axiom(Lecture* closure, std::string source)
        : Lecture(closure), data(parse(this, source))
{
    if (data->getType() != logical_mt)
        throw std::invalid_argument("Аксиома должна быть логического типа.\n");
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
    size_t buf = 0;
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

std::string AbstrInf::getTypeAsStr() const
{
    switch (type)
    {
        case InfTy::MP   : return "InfMP";
        case InfTy::GEN  : return "InfGen";
        case InfTy::SPEC : return "InfSpec";
    }
}


Terms* modusPonens(const Terms* premise, const Terms* impl)
{
    if (const auto* tI = dynamic_cast<const Term*>(impl))
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
InfMP::InfMP(Lecture* closure, Path pArg1, Path pArg2)
        : AbstrInf(closure, AbstrInf::InfTy::MP, pArg1, pArg2),
          data(modusPonens(getParent()->getTerms(pArg1), getParent()->getTerms(pArg2)))
{ if (!data) throw bad_inf(); }

Terms* specialization(const Terms* general, const Terms* t)
{
    if (const auto* fT = dynamic_cast<const ForallTerm*>(general))
    {
        if (fT->arg(1)->getType() == t->getType())
            return fT->arg(2)->replace(fT->arg(1), t);
    }
    else
        return nullptr;
}
InfSpec::InfSpec(Lecture* closure, Path pArg1, Path pArg2)
        : AbstrInf(closure, AbstrInf::InfTy::SPEC, pArg1, pArg2),
          data(specialization(getParent()->getTerms(pArg1), getParent()->getTerms(pArg2)))
{ if (!data) throw bad_inf(); }

Term* generalization  (const Terms* toGen, const Terms* x)
{
    if (const auto* v = dynamic_cast<const Variable*>(x))
    {
        if (const auto* tG = dynamic_cast<const Term*>(toGen))
        if (tG->free.find(*v) != tG->free.end())
            return new ForallTerm(*v, tG->clone());
    }
    else
        return nullptr;
}
InfGen::InfGen(Lecture* closure, Path pArg1, Path pArg2)
        : AbstrInf(closure, AbstrInf::InfTy::GEN, pArg1, pArg2),
          data(generalization(getParent()->getTerms(pArg1), getParent()->getTerms(pArg2)))
{ if (!data) throw bad_inf(); }


Hierarchy* Hierarchy::fromJson(const json& j, Lecture* parent)
{
    const std::string itemType = j.at("ItemType");
    if (itemType == "Lecture")
        return Lecture::fromJson(j.at("ItemData"), parent);
    else if (itemType == "DefType")
        return DefType::fromJson(j.at("ItemData"), parent);
    else if (itemType == "DefVar")
        return DefVar::fromJson(j.at("ItemData"), parent);
    else if (itemType == "DefSym")
        return DefSym::fromJson(j.at("ItemData"), parent);
    else if (itemType == "Axiom")
        return Axiom::fromJson(j.at("ItemData"), parent);
    else if (itemType == "InfMP")
        return InfMP::fromJson(j.at("ItemData"), parent);
    else if (itemType == "InfSpec")
        return InfSpec::fromJson(j.at("ItemData"), parent);
    else if (itemType == "InfGen")
        return InfGen::fromJson(j.at("ItemData"), parent);
    else
        return nullptr;
}
Hierarchy* Lecture::fromJson(const json& j, Lecture* parent)
{
    auto section = new Lecture(parent, j.at("title"));
    json subs = j.at("subs");
    for (const auto& s : subs)
        Hierarchy::fromJson(s, section);
    return section;
}
Hierarchy* DefType::fromJson(const json& j, Lecture* parent)
{ return new DefType(parent, j.at("name")); }
Hierarchy* DefVar::fromJson(const json& j, Lecture* parent)
{
    auto type = parent->index().getT(j.at("type"));
    return new DefVar(parent, j.at("name"), type);
}
Hierarchy* DefSym::fromJson(const json& j, Lecture* parent)
{
    std::list<MathType> argT;
    auto index = parent->index();
    for (const auto& t : j.at("argT"))
        argT.push_back(index.getT(t));
    MathType retT = parent->index().getT(j.at("retT"));
    return new DefSym(parent, j.at("name"), argT, retT);
}
Hierarchy* Axiom::fromJson(const json& j, Lecture* parent)
{ return new Axiom(parent, j.at("axiom")); }
Hierarchy* InfMP::fromJson(const json& j, Lecture* parent)
{ return new InfMP(parent, mkPath(j.at("arg1")), mkPath(j.at("arg2"))); }
Hierarchy* InfSpec::fromJson(const json& j, Lecture* parent)
{ return new InfSpec(parent, mkPath(j.at("arg1")), mkPath(j.at("arg2"))); }
Hierarchy* InfGen::fromJson(const json& j, Lecture* parent)
{ return new InfGen(parent, mkPath(j.at("arg1")), mkPath(j.at("arg2"))); }


std::string Hierarchy::toString() const
{
    size_t n = 1;
    std::stringstream buf;
    for (const auto& s : subs)
        buf << '(' << n++ << "): " << s->toString() << std::endl;
    return buf.str();
}
std::string Lecture::toString() const
{ return ("Раздел \"" + getTitle() + "\".\n"); }
std::string DefType::toString() const
{ return ("Объявлен тип " + getName() + "."); }
std::string DefVar::toString() const
{ return ("Добавлена переменная " + getName() +
            " типа " + getType().getName() + "."); }
std::string DefSym::toString() const
{
    std::stringstream buf;
    buf << "Введен символ " << getName() << " : ";
    auto argTypes = getSign().first;
    if (!argTypes.empty())
    {
        buf << argTypes.front().getName();
        auto e = argTypes.end();
        for (auto it = next(argTypes.begin()); it != e; ++it)
            buf << " x " << it->getName();
    }
    buf << " -> " << getType().getName() << ".";
    return buf.str();
}
std::string Axiom::toString() const
{
    std::stringstream buf;
    buf << "Пусть ";
    get()->print(buf);
    return buf.str();
}
std::string AbstrInf::toString() const
{
    std::stringstream buf;
    buf << "По ";
    switch (type)
    {
        case InfTy::MP :
            buf << "MP ";   break;
        case InfTy::SPEC :
            buf << "Spec "; break;
        case InfTy::GEN :
            buf << "Gen ";  break;
    }
    buf << "из " << pathToStr(*premises.begin()) << " и "
        << pathToStr(*std::next(premises.begin())) << " следует: ";
    get()->print(buf);
    return buf.str();
}


json Hierarchy::toJson() const
{
    json temp;
    for (const auto& s : subs)
        temp.push_back(s->toJson());
    return temp;
}
json Lecture::toJson() const
{
    json temp;
    temp["ItemType"] = "Lecture";
    temp["ItemData"] = { {"title", title},
                         {"subs", Hierarchy::toJson()} };
    return temp;
}
json DefType::toJson() const
{
    json temp;
    temp["ItemType"] = "DefType";
    temp["ItemData"] = { {"name", getName()} };
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
    temp["ItemData"] = { {"axiom", ss.str()}/*,
                         {"subs", Hierarchy::toJson()}*/ };
    return temp;
}
json AbstrInf::toJson() const
{
    json temp;
    switch (type)
    {
        case InfTy::MP : { temp["ItemType"] = "InfMP"; break; }
        case InfTy::GEN : { temp["ItemType"] = "InfGen"; break; }
        case InfTy::SPEC : { temp["ItemType"] = "InfSpec"; break; }
    }
    temp["ItemData"] = { {"arg1", pathToStr(premises.at(0))},
                         {"arg2", pathToStr(premises.at(1))} };
    return temp;
}


struct MlObj
{
    std::string mlType;
    size_t label;
    std::string body;
    std::vector<Path> premises;

    MlObj(std::string _mlType, size_t _label,
          std::string _body, std::vector<Path> _premises = {}) :
            mlType(std::move(_mlType)), label(_label),
            body(std::move(_body)), premises(std::move(_premises)) {}

    json toJson()
    {
        return json({ {"mlType", mlType},
                      {"label", {label}},
                      {"body", body},
                      {"premises", premises} });
    }
};
json Lecture::toMlObj() const
{ return MlObj("section", 0, getTitle()).toJson(); }
json DefType::toMlObj() const
{ return MlObj("def_type", getNumber(), getName()).toJson(); }
json DefVar::toMlObj() const
{
    std::stringstream body;
    body << getName() << "\\in " << getType().getName();
    return MlObj("def_var", getNumber(), body.str()).toJson();
}
json DefSym::toMlObj() const
{
    std::stringstream out;
    out << getName() << " : ";
    auto argTypes = getSign().first;
    if (!argTypes.empty())
    {
        out << argTypes.front().getName();
        auto e = argTypes.end();
        for (auto it = next(argTypes.begin()); it != e; ++it)
            out << "\\times " << it->getName();
    }
    out << "\\rightarrow " << getType().getName();
    return MlObj("def_sym", getNumber(), out.str()).toJson();
}
json Axiom::toMlObj() const
{
    std::stringstream ax; ax << *data;
    return MlObj("axiom", getNumber(), ax.str()).toJson();
}
json AbstrInf::toMlObj() const
{
    std::string infType;
    switch (type)
    {
        case InfTy::MP   : { infType = "inf_mp";  break; }
        case InfTy::GEN  : { infType = "inf_gen"; break; }
        case InfTy::SPEC : { infType = "inf_spec";break; }
    }
    std::stringstream inf; inf << *get();
    return MlObj(infType, getNumber(), inf.str(), premises).toJson();
}
