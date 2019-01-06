//
// Created by anton on 28.09.18.
//

#include "mapterm.hpp"

Map::Map(const NamesType& symForm, ProductMT argT, const MathType* retT)
    : NamedTerm(symForm), _type(argT, retT) {
    if (argT.getArity() == 0)
        throw std::invalid_argument("Нуль-арные отображения запрещены. Используйте константы.");
    unsigned argPlaces = std::count_if(symForm.begin(), symForm.end(),
                                       [] (const Parser2::Lexeme& l) -> bool {
        auto lexCat = Parser2::texLexer.storage.which(l._id);
        return (lexCat == "argument_place" || lexCat == "variable_place");
    });
    if (argPlaces != argT.getArity())
        throw std::invalid_argument("Кол-во типов аргументов (" + std::to_string(argT.getArity()) +
                                    ") не соответствует форме символа (" + Parser2::texLexer.print(symForm) + ")");
}

bool Map::operator<(const Map& other) const {
    if (_type != other._type)
        return (_type < other._type);
    else
        return (this->NamedTerm::operator<)(other);
}
