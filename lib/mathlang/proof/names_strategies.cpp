//
// Created by anton on 19.09.18.
//

#include "names_strategies.hpp"

NameStoringStrategy* nssFromStr(std::string str, Node* parent) {
    if (str == "Hidden")
        return new Hidden(parent);
    else
        return new Appending(parent);
}

NameStoringStrategy* nssFromNSSType(NssType type, Node* parent) {
    switch (type) {
        case NssType::Hidden :
            return new Hidden(parent);
        case NssType::Appending :
            return new Appending(parent);
        default:
            return nullptr;
    }
}