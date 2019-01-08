//
// Created by anton on 07.01.19.
//

#include "texname.hpp"
#include "../parser/lexer.hpp"

void TexName::fromStr(const std::string& name) {
    Parser2::CurAnalysisData cad = Parser2::texLexer.recognize(name);
    if (!cad.res.success)
        throw Parser2::parse_error(cad.res);
    _nameSeq = cad.filtered;
}

std::string TexName::toStr() const
{ return Parser2::texLexer.print(_nameSeq); }
