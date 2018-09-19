//
// Created by anton on 05.09.18.
//

#ifndef SPIKARD_PARSER2_HPP
#define SPIKARD_PARSER2_HPP

#include <vector>

#include "lexer.hpp"

namespace Parser2
{

struct PartialResolved
{
    typedef std::vector<LexemeSequence> result_type;

    size_t indent;
    result_type recognized;

    PartialResolved(size_t indent, result_type recognized)
        : indent(indent), recognized(std::move(recognized)) {}
    bool operator< (const PartialResolved& two) const
    { return (indent < two.indent); }
};

class Parser
{
public:
    const PrimaryNode* _where;
    Hidden localNames;
    std::set<std::string> namesDefined;
    std::set<LexemeSequence> definedTexSeq;

    Parser(PrimaryNode* where);
};

}

#endif //SPIKARD_PARSER2_HPP
