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