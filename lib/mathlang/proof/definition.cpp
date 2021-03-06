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
    type = new PrimaryType(name);
    parent->registerNamed(type, this);
}
PrimaryType* DefType::use(Item* in) {
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
    parent->registerNamed(atom, this);
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
    parent->registerNamed(funct, this);   // todo подавать тип при вызове больше не нужно
    // todo двойная регистрация: с возвращаемым типом как связки и как переменной функционального типа
}
Function* DefFunct::use(Item* in) {
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
    parent->registerNamed(connective, this);
}
DefConnective::DefConnective(Node* parent, const std::string& sym, BinaryOperation::Notation notation,
    DefType* leftT, DefType* rightT, DefType* retT)
    : Item(parent) {
    auto leftType = leftT->use(this);
    auto rightType = rightT->use(this);
    auto retType = retT->use(this);
    auto* name = new TexName(sym);
    connective = new BinaryOperation(name, leftType, rightType, retType, notation);
    parent->registerNamed(connective, this);
}
DefConnective::DefConnective(Node* parent, const std::string& form, const std::vector<DefType*>& argT, DefType* retT)
    : Item(parent) {
    MathType::Vector argTypes;
    for (auto* d : argT)
        argTypes.push_back(d->use(this));
    auto retType = retT->use(this);
    auto* name = new TexName(form);
    connective = new SpecialConnective(name, argTypes, retType);
    parent->registerNamed(connective, this);
}
NamedEntity* DefConnective::use(Item* in) {
    addUsage(in);
    return _get();
}
std::string DefConnective::print(Representation* r, bool incremental) const
{ r->process(this); return r->str(); }
DefConnective::~DefConnective()
{ delete connective; }
