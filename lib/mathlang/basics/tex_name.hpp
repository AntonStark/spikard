//
// Created by anton on 07.01.19.
//

#ifndef SPIKARD_TEXNAME_HPP
#define SPIKARD_TEXNAME_HPP

#include <typeinfo>

#include "../consepts/abstract_name.hpp"

#include "../parser/lexeme.hpp"
#include "../parser/lexer.hpp"

class TexName : public AbstractName
{
private:
    Parser2::CurAnalysisData _data;
    Parser2::LexemeSequence _lexSeq;
    void fromStr(const std::string& name) override;
public:
    TexName(const std::string& name, bool noBlanks = false);
    TexName(Parser2::LexemeSequence lexSeq);

    std::string toStr() const override;

    bool operator==(const AbstractName& other) const override;

    long countArgPlaces() const;
    const Parser2::LexemeSequence& getSeq() const;
};

extern Parser2::Lexer texLexer;

#endif //SPIKARD_TEXNAME_HPP
