
//
// Created by anton on 07.01.19.
//

#include "tex_name.hpp"

void TexName::fromStr(const std::string& name) {
    _data = texLexer.recognize(name);
    if (!_data.res.success)
        throw Parser2::parse_error(_data.res);
    _lexSeq = _data.filtered;
}

std::string TexName::toStr() const
{ return texLexer.print(_lexSeq); }

TexName::TexName(const std::string& name, bool noBlanks) {
    fromStr(name);
    if (noBlanks) {
        if (_data.blankFound)
            throw Parser2::parse_error("имя не должно содержать команд отступа.");
    }
}

TexName::TexName(Parser2::LexemeSequence lexSeq)
: _lexSeq(std::move(lexSeq)) {}

long TexName::countArgPlaces() const {
    return std::count_if(_data.filtered.begin(), _data.filtered.end(),
        [](const Parser2::Lexeme& l) -> bool {
        auto lexCat = texLexer.storage.which(l._id);
        return (lexCat == "argument_place" || lexCat == "variable_place");
    });
}

const Parser2::LexemeSequence& TexName::getSeq() const
{ return _lexSeq; }
bool TexName::operator==(const AbstractName& other) const {
    try {
        auto otherTexName = dynamic_cast<const TexName&>(other);
        return (getSeq() == otherTexName.getSeq());
    } catch (std::bad_cast&)
    { return false; }
}

Parser2::Lexer texLexer = Parser2::Lexer::configureLatex();
