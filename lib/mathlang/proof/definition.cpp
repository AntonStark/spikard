//
// Created by anton on 10.01.17.
//

#include "definition.hpp"

PrimaryMT* getType(const NameSpaceIndex& index, const std::string& name) {
    auto* texName = new TexName(name);
    return index.get(texName)->getType(); // fixme использовать запрос имён типа Type когда будет реализовано
}

Hierarchy* Definition::fromJson(const json& j, Node* parent, NameTy type) {
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
}
