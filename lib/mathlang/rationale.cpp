//
// Created by anton on 10.01.17.
//

#include "rationale.hpp"

#include <utility>

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

void PrimaryNode::addTerm(const std::string& term)
{ new TermsBox(this, term); }
void PrimaryNode::doMP  (const std::string& pPremise, const std::string& pImpl)
{ new Inference(this, mkPath(pPremise), mkPath(pImpl), Inference::InfTy::MP); }
void PrimaryNode::doSpec(const std::string& pToSpec, const std::string& termVar)
{
    Path pTerm = mkPath(termVar);
    if (pTerm.empty()) {
        addTerm(termVar);
        pTerm = backLabel();
    }
    new Inference(this, mkPath(pToSpec), pTerm, Inference::InfTy::SPEC);
}
void PrimaryNode::doGen (const std::string& pToGen,  const std::string& pToVar)
{ new Inference(this, mkPath(pToGen), mkPath(pToVar), Inference::InfTy::GEN); }


TermsBox::TermsBox(PrimaryNode* parent, std::string source)
        : PrimaryNode(parent, new Hidden(parent), NamedNodeType::CLOSURE, ""),
          data(parse(this, std::move(source))) {}

class Inference::bad_inf : public std::invalid_argument
{
public:
    bad_inf(const std::string& args)
            : std::invalid_argument("Неподходящие аргументы " + args +
                                    " для данного вывода.") {}
};

const Terms* Inference::getTerms(Path pathToTerm) {
    Hierarchy* termItem = getParent()->getByPass(pathToTerm);
    if (auto t = dynamic_cast<Statement*>(termItem))
        return t->get();
    else if (auto v = dynamic_cast<DefVar*>(termItem))
        return v;
    else
        return nullptr;
}
const Terms* Inference::inference() {
    const Terms* arg1 = getTerms(premises[0]);
    const Terms* arg2 = getTerms(premises[1]);
    switch (type) {
        case Inference::InfTy::MP   :
            return modusPonens   (arg1, arg2);
        case Inference::InfTy::SPEC :
            return specialization(arg1, arg2);
        case Inference::InfTy::GEN  :
            return generalization(arg1, arg2);
    }
}
Inference::Inference(PrimaryNode* naming,
                     Path pArg1, Path pArg2, Inference::InfTy _type)
        : Item(naming), premises({std::move(pArg1), std::move(pArg2)}),
          type(_type), data(inference())
{ if (!data) throw bad_inf(pathToStr(pArg1) + ", " + pathToStr(pArg2)); }

std::string Inference::getTypeAsStr() const {
    switch (type)
    {
        case InfTy::MP   : return "InfMP";
        case InfTy::GEN  : return "InfGen";
        case InfTy::SPEC : return "InfSpec";
    }
}
Inference::InfTy infTyFromStr(const std::string& type) {
    if (type == "InfMP")
        return Inference::InfTy::MP;
    else if (type == "InfSpec")
        return Inference::InfTy::SPEC;
    else
        return Inference::InfTy::GEN;
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

Terms* specialization(const Terms* general, const Terms* t) {
    if (const auto* fT = dynamic_cast<const ForallTerm*>(general)) {
        if (fT->arg(1)->getType() == t->getType())
            return fT->arg(2)->replace(fT->arg(1), t);
    }
    else
        return nullptr;
}

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
        else if (type == "TermsBox")
            TermsBox::  fromJson(data, pn);
        else if (type == "Inference")
            Inference::fromJson(data, pn);
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

Hierarchy* TermsBox::fromJson(const json& j, PrimaryNode* parent)
{ return new TermsBox(parent, j.at("axiom")); }

Hierarchy* Inference::fromJson(const json& j, PrimaryNode* parent)
{ return new Inference(parent, mkPath(j.at("premise1")),
                       mkPath(j.at("premise2")), infTyFromStr(j.at("type"))); }
