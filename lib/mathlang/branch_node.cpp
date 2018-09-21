//
// Created by anton on 21.09.18.
//

#include "branch_node.hpp"

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
