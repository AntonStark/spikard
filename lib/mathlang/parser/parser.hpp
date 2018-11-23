//
// Created by anton on 05.09.18.
//

#ifndef SPIKARD_PARSER_HPP
#define SPIKARD_PARSER_HPP

#include <vector>

#include "lexer.hpp"
#include "../proof/named_node.hpp"

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
    const Node* _where;
    std::set<LexemeSequence> namesDefined;

    Parser(Node* where);

    Terms* parse(CurAnalysisData& source);
};

Terms* parse(Node* where, std::string source);
}

#endif //SPIKARD_PARSER_HPP
