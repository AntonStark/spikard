//
// Created by anton on 28.09.18.
//

#include "mapterm.hpp"

Map::Map(const AbstractName* symForm, ProductMT argT, const MathType* retT)
    : NamedTerm(symForm), _type(argT, retT) {
    if (argT.getArity() == 0)
        throw std::invalid_argument("Нуль-арные отображения запрещены. Используйте константы.");
    if (const auto* texName = dynamic_cast<const TexName*>(symForm)) {
        unsigned argPlaces = texName->countArgPlaces();
        if (argPlaces != argT.getArity())
            throw std::invalid_argument("Кол-во типов аргументов (" + std::to_string(argT.getArity()) +
                                        ") не соответствует форме символа (" + symForm->toStr() + ")");
    }
}

bool Map::operator<(const Map& other) const {
    if (_type != other._type)
        return (_type < other._type);
    else
        return (this->NamedTerm::operator<)(other);
}
