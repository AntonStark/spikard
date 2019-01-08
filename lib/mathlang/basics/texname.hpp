#include <utility>

//
// Created by anton on 07.01.19.
//

#ifndef SPIKARD_TEXNAME_HPP
#define SPIKARD_TEXNAME_HPP

#include "namestype.hpp"
#include "../parser/lexeme.hpp"

class TexName : public NamesType
{
private:
    Parser2::LexemeSequence _nameSeq;
    void fromStr(const std::string& name) override;
public:
    TexName(const std::string& name) { fromStr(name); }
    std::string toStr() const override;
};

#endif //SPIKARD_TEXNAME_HPP
