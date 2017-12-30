//
// Created by anton on 10.01.17.
//

#include "rationale.hpp"

MathType getType(const NameSpaceIndex& index, const std::string& name)
{ return *dynamic_cast<DefType*>(*index.get(NameTy::MT, name).begin()); }
Variable getVar (const NameSpaceIndex& index, const std::string& name)
{ return *dynamic_cast<DefVar*>(*index.get(NameTy::VAR, name).begin()); }
std::set<Symbol> getSym(const NameSpaceIndex& index, const std::string& name) {
    std::set<Symbol> buf;
    for (const auto& def : index.get(NameTy::SYM, name))
        buf.insert(*dynamic_cast<DefSym*>(def));
    return buf;
}


std::string toStr(NamedNodeType nnt) {
    switch (nnt) {
        case NamedNodeType::COURSE  : return "Курс" ;
        case NamedNodeType::SECTION : return "Раздел";
        case NamedNodeType::LECTURE : return "Лекция";
        case NamedNodeType::CLOSURE : return "Замыкание";
    }
}
NamedNodeType nntFromStr(std::string str) {
    if (str == "Курс")
        return NamedNodeType::COURSE;
    else if (str == "Раздел")
        return NamedNodeType::SECTION;
    else if (str == "Лекция")
        return NamedNodeType::LECTURE;
    else
        return NamedNodeType::CLOSURE;
}


void PrimaryNode::defType(const std::string& typeName)
{ new DefType(this, typeName); }
void PrimaryNode::defVar(const std::string& varName, const std::string& typeName)
{ new DefVar(this, varName, getType(index(), typeName)); }
void PrimaryNode::defSym(
        const std::string& symName, const std::list<std::string>& argT,
        const std::string& retT) {
    std::list<MathType> argMT;
    for (auto& a : argT)
        argMT.push_back(getType(index(), a));
    new DefSym(this, symName, argMT, getType(index(), retT));
}

void PrimaryNode::addAxiom(const std::string& axiom)
{ new Axiom(this, axiom); }
void PrimaryNode::doMP  (const std::string& pPremise, const std::string& pImpl)
{ new InfMP(this, mkPath(pPremise), mkPath(pImpl)); }
void PrimaryNode::doSpec(const std::string& pToSpec, const std::string& termVar)
{ new InfSpec(this, mkPath(pToSpec), parse(this, termVar)); }
void PrimaryNode::doGen (const std::string& pToGen,  const std::string& pToVar)
{ new InfGen(this, mkPath(pToGen), mkPath(pToVar)); }


Axiom::Axiom(PrimaryNode* parent, std::string source)
        : PrimaryNode(parent, new Hidden(parent), NamedNodeType::CLOSURE, ""),
          data(parse(this, source)) {
    if (data->getType() != logical_mt)
        throw std::invalid_argument("Аксиома должна быть логического типа.\n");
}

class AbstrInf::bad_inf : public std::invalid_argument
{
public:
    bad_inf() : std::invalid_argument("Неподходящие аргументы для данного вывода.") {}
};

const Terms* AbstrInf::getTerms(Path pathToTerm) {
    Hierarchy* termItem = getParent()->getByPass(pathToTerm);
    if (auto t = dynamic_cast<Statement*>(termItem))
        return t->get();
    else if (auto v = dynamic_cast<DefVar*>(termItem))
        return v;
    else
        return nullptr;
}
std::string AbstrInf::getTypeAsStr() const {
    switch (type)
    {
        case InfTy::MP   : return "InfMP";
        case InfTy::GEN  : return "InfGen";
        case InfTy::SPEC : return "InfSpec";
    }
}


Terms* modusPonens(const Terms* premise, const Terms* impl) {
    if (const auto* tI = dynamic_cast<const Term*>(impl)) {
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
InfMP::InfMP(PrimaryNode* naming, Path pPremise, Path pImpl)
        : AbstrInf(naming, AbstrInf::InfTy::MP, pPremise, pImpl),
          data(modusPonens(getTerms(pPremise), getTerms(pImpl)))
{ if (!data) throw bad_inf(); }

Terms* specialization(const Terms* general, const Terms* t) {
    if (const auto* fT = dynamic_cast<const ForallTerm*>(general)) {
        if (fT->arg(1)->getType() == t->getType())
            return fT->arg(2)->replace(fT->arg(1), t);
    }
    else
        return nullptr;
}
InfSpec::InfSpec(PrimaryNode* naming, Path pGeneral, Terms* tCase)
    : AbstrInf(naming, AbstrInf::InfTy::SPEC, pGeneral),
      data(specialization(getTerms(pGeneral), tCase)), spec(tCase)
{ if (!data) throw bad_inf(); }

Term* generalization(const Terms* toGen, const Terms* x) {
    if (const auto* v = dynamic_cast<const Variable*>(x)) {
        if (const auto* tG = dynamic_cast<const Term*>(toGen)) {
            if (tG->free.find(*v) != tG->free.end())
                return new ForallTerm(*v, tG->clone());
        }
    }
    else
        return nullptr;
}
InfGen::InfGen(PrimaryNode* naming, Path pArg1, Path pArg2)
        : AbstrInf(naming, AbstrInf::InfTy::GEN, pArg1, pArg2),
          data(generalization(getTerms(pArg1), getTerms(pArg2)))
{ if (!data) throw bad_inf(); }


PrimaryNode* PrimaryNode::fromJson(const json& j, BranchNode* parent) {
    auto* pn = new PrimaryNode(parent, nssFromStr(j.at("storing_strategy"), parent),
                              nntFromStr(j.at("type")), j.at("title"));
    json jsubs = j.at("subs");
    for (const auto& s : jsubs) {
        auto type = s.at(0);
        auto data = s.at(1);
        if (type == "DefType")
            DefType::fromJson(data, pn);
        else if (type == "DefVar")
            DefVar:: fromJson(data, pn);
        else if (type == "DefSym")
            DefSym:: fromJson(data, pn);
        else if (type == "Axiom")
            Axiom::  fromJson(data, pn);
        else if (type == "InfMP")
            InfMP::  fromJson(data, pn);
        else if (type == "InfSpec")
            InfSpec::fromJson(data, pn);
        else if (type == "InfGen")
            InfGen:: fromJson(data, pn);
    }
    return pn;
}

BranchNode* BranchNode::fromJson(const json& j, BranchNode* parent) {
    auto* bn = new BranchNode(parent, nssFromStr(j.at("storing_strategy"), parent),
                              nntFromStr(j.at("type")), j.at("title"));
    json jsubs = j.at("subs");
    for (const auto& s : jsubs) {
        auto o = s.at(1);
        if (o.at("type") == "Курс" || o.at("type") == "Раздел")
            BranchNode ::fromJson(o, bn);
        else if (o.at("type") == "Лекция" || o.at("type") == "Замыкание")
            PrimaryNode::fromJson(o, bn);
    }
    return bn;
}

Hierarchy* DefType::fromJson(const json& j, PrimaryNode* parent)
{ return new DefType(parent, j.at("name")); }

Hierarchy* DefVar::fromJson(const json& j, PrimaryNode* parent) {
    auto type = ::getType(parent->index(), j.at("type"));
    return new DefVar(parent, j.at("name"), type);
}

Hierarchy* DefSym::fromJson(const json& j, PrimaryNode* parent) {
    std::list<MathType> argT;
    auto index = parent->index();
    for (const auto& t : j.at("argT"))
        argT.push_back(::getType(index, t));
    MathType retT = ::getType(parent->index(), j.at("retT"));
    return new DefSym(parent, j.at("name"), argT, retT);
}

Hierarchy* Axiom::fromJson(const json& j, PrimaryNode* parent)
{ return new Axiom(parent, j.at("axiom")); }
Hierarchy* InfMP::fromJson(const json& j, PrimaryNode* parent)
{ return new InfMP(parent, mkPath(j.at("premise")), mkPath(j.at("impl"))); }
Hierarchy* InfSpec::fromJson(const json& j, PrimaryNode* parent)
{ return new InfSpec(parent, mkPath(j.at("general")), parse(parent, j.at("case"))); }
Hierarchy* InfGen::fromJson(const json& j, PrimaryNode* parent)
{ return new InfGen(parent, mkPath(j.at("arg1")), mkPath(j.at("arg2"))); }
