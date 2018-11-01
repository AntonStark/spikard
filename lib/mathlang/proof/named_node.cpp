//
// Created by anton on 21.09.18.
//

#include "named_node.hpp"

std::string NamedNode::typeToStr(NNType nnt) {
    switch (nnt) {
        case NNType::COURSE  : return "Курс" ;
        case NNType::SECTION : return "Раздел";
        case NNType::LECTURE : return "Лекция";
        case NNType::CLOSURE : return "Замыкание";
    }
}
NamedNode::NNType NamedNode::nntFromStr(std::string str) {
    if (str == "Курс")
        return NNType::COURSE;
    else if (str == "Раздел")
        return NNType::SECTION;
    else if (str == "Лекция")
        return NNType::LECTURE;
    else if (str == "Замыкание")
        return NNType::CLOSURE;
    else
        throw std::invalid_argument(str + " - не название типа NamedNode."
                                          "Допустимые строки: Курс, Раздел, Лекция, Замыкание.");
}

void PrimaryNode::defType(const std::string& typeName)
{ new Definition(this, typeName); }
void PrimaryNode::defConst(const std::string& constName, const std::string& typeName)
{ new Definition(this, NameTy::CONST, constName, getType(index(), typeName)); }
void PrimaryNode::defVar(const std::string& varName, const std::string& typeName)
{ new Definition(this, NameTy::VAR,   varName, getType(index(), typeName)); }
void PrimaryNode::defSym(
    const std::string& symForm, const std::vector<std::string>& argT,
    const std::string& retT) {
    std::vector<const MathType*> argMT;
    for (auto& a : argT)
        argMT.push_back(getType(index(), a));
    new Definition(this, symForm, argMT, getType(index(), retT));
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
void PrimaryNode::doApply(const std::string& pTerm, const std::string& pTheorem)
{ new Inference(this, mkPath(pTerm), mkPath(pTheorem), Inference::InfTy::APPL); }
void PrimaryNode::doEqual(const std::string& pTerm, const std::string& pEquality)
{ new Inference(this, mkPath(pTerm), mkPath(pEquality), Inference::InfTy::EQL); }
void PrimaryNode::doGen (const std::string& pToGen,  const std::string& pToVar)
{ new Inference(this, mkPath(pToGen), mkPath(pToVar), Inference::InfTy::GEN); }


PrimaryNode* PrimaryNode::fromJson(const json& j, Node* parent) {
    auto* pn = new PrimaryNode(parent, nssFromStr(j.at("storing_strategy"), parent),
                               j.at("type").get<std::string>(), j.at("title"));
    json jsubs = j.at("subs");
    for (const auto& s : jsubs) {
        auto type = s.at(0);
        auto data = s.at(1);
        if (type == "TermsBox")
            TermsBox::  fromJson(data, pn);
        else if (type == "Inference")
            Inference::fromJson(data, pn);
        else {
            NameTy nt;
            if (type == "DefType")
                nt = NameTy::MT;
            else if (type == "DefVar")
                nt = NameTy::VAR;
            else if (type == "DefConst")
                nt = NameTy::CONST;
            else if (type == "DefSym")
                nt = NameTy::SYM;
            Definition::fromJson(data, pn, nt);
        }
    }
    return pn;
}

BranchNode* BranchNode::fromJson(const json& j, BranchNode* parent) {
    auto* bn = new BranchNode(parent, nssFromStr(j.at("storing_strategy"), parent),
                              j.at("type").get<std::string>(), j.at("title"));
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