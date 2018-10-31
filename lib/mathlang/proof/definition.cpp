//
// Created by anton on 10.01.17.
//

#include "definition.hpp"

// NB вообще можно вводить сущность с тем же именем (перегрузка), если ещё нет идентичной сущности:
// для типов и переменных (и констант) это означает однозначность определения по имени, для символов - по имени и сигнатуре
PrimaryMT* getType(const NameSpaceIndex& index, const std::string& name) {
    Parser2::CurAnalysisData cad = Parser2::texLexer.recognize(name);
    return index.get(NameTy::MT, cad.lexems)->getType();
}
/*NamedTerm* getVar (const NameSpaceIndex& index, const std::string& name)
{ return index.get(NameTy::VAR, name)->getTerm(); }
NamedTerm* getConst(const NameSpaceIndex& index, const std::string& name)
{ return index.get(NameTy::CONST, name)->getTerm(); }
NamedTerm* getSym(const NameSpaceIndex& index, const std::string& name)
{ return index.get(NameTy::SYM, name)->getTerm(); }*/
// todo эти методы действительно нужны новому парсеру?

Hierarchy* Definition::fromJson(const json& j, Node* parent, NameTy type) {
    switch (type) {
        case NameTy::MT : {
            return new Definition(parent, j.at("name"));
        }
        case NameTy::VAR : {
            auto type = ::getType(parent->index(), j.at("type"));
            return new Definition(parent, NameTy::VAR, j.at("name"), type);
        }
        case NameTy::CONST : {
            auto type = ::getType(parent->index(), j.at("type"));
            return new Definition(parent, NameTy::CONST, j.at("name"), type);
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
}
