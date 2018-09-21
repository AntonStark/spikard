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
