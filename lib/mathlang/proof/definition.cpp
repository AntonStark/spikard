//
// Created by anton on 10.01.17.
//

#include "definition.hpp"

/*Hierarchy* Definition::fromJson(const json& j, Node* parent, NameTy type) {
    switch (type) {
        case NameTy::MT : {
            return new Definition(parent, j.at("name"));
        }
        case NameTy::VAR : {
            auto type = ::getType(parent->index(), j.at("type"));
            return new Definition(parent, j.at("name"), type);
        }
        case NameTy::SYM : {
            std::vector<const MathType*> argT;
            auto index = parent->index();
            for (const auto& t : j.at("argT"))
                argT.push_back(::getType(index, t));
            MathType* retT = ::getType(parent->index(), j.at("retT"));
            return new Definition(parent, j.at("name"), argT, retT);
        }
    }
}*/

DefType::DefType(Node* parent, const std::string& typeName)
    : Item(parent) {
    auto* name = new TexName(typeName, true);
    type = new PrimaryMT(name);
    parent->registerNamedTerm(type, this);
}
PrimaryMT* DefType::use(Item* in) {
    addUsage(in);
    return _get();
}
std::string DefType::print(Representation* r, bool incremental) const
{ r->process(this); return r->str(); }
DefType::~DefType()
{ delete type; }

DefAtom::DefAtom(Node* parent, const std::string& varName, DefType* mathType)
    : Item(parent) {
    auto* name = new TexName(varName, true);
    auto* type = mathType->use(this);
    atom = new Variable(name, type);
    parent->registerNamedTerm(atom, this);
}
Variable* DefAtom::use(Item* in) {
    addUsage(in);
    return _get();
}
std::string DefAtom::print(Representation* r, bool incremental) const
{ r->process(this); return r->str(); }
DefAtom::~DefAtom()
{ delete atom; }

DefFunct::DefFunct(Node* parent, const std::string& fName, DefType* argT, DefType* retT)
    : Item(parent) {
    auto argType = argT->use(this);
    auto retType = retT->use(this);
    auto* name = new TexName(fName);
    funct = new Function(name, argType, retType);
    parent->registerNamedTerm(funct, this);
}
Map* DefFunct::use(Item* in) {
    addUsage(in);
    return _get();
}
std::string DefFunct::print(Representation* r, bool incremental) const
{ r->process(this); return r->str(); }
DefFunct::~DefFunct()
{ delete funct; }

DefConnective::DefConnective(Node* parent, const std::string& sym, bool prefix,
    DefType* argT, DefType* retT)
    : Item(parent) {
    auto argType = argT->use(this);
    auto retType = retT->use(this);
    auto* name = new TexName(sym);
    connective = new UnaryOperation(name, argType, retType, prefix);
}
DefConnective::DefConnective(Node* parent, const std::string& sym, BinaryOperation::Notation notation,
    DefType* leftT, DefType* rightT, DefType* retT)
    : Item(parent) {
    auto leftType = leftT->use(this);
    auto rightType = rightT->use(this);
    auto retType = retT->use(this);
    auto* name = new TexName(sym);
    connective = new BinaryOperation(name, leftType, rightType, retType, notation);
}
DefConnective::~DefConnective() { delete connective; }
std::string DefFunct::print(Representation* r, bool incremental) const
{ r->process(this); return r->str(); }
