//
// Created by anton on 28.09.18.
//

#include "mapterm.hpp"

Map::Map(const std::string& name, ProductMT argT, const MathType* retT)
    : NamedTerm(name), _type(argT, retT) {
    if (argT.getArity() == 0)
        throw std::invalid_argument("Нуль-арные отображения запрещены. Используйте константы.");
}

bool Map::operator<(const Map& other) const {
    if (_type != other._type)
        return (_type < other._type);
    else
        return (this->NamedTerm::operator<)(other);
}

class Map::argN_oper_error : public std::invalid_argument
{
public:
    argN_oper_error()
        : std::invalid_argument("Кол-во типов аргументов не соответствует типу операции.") {}
};

UnaryOperation::UnaryOperation(const std::string& name, Form form,
                               ProductMT argT, const MathType* retT)
    : Map(name, argT, retT), _form(form) { if (argT.getArity() != 1) throw argN_oper_error(); }

BinaryOperation::BinaryOperation(const std::string& name, Form form,
                                 ProductMT argT, const MathType* ret)
    : Map(name, argT, ret), _form(form) { if (argT.getArity() != 2) throw argN_oper_error(); }

TernaryOperation::TernaryOperation(const std::string& name, ProductMT argT, const MathType* ret)
    : Map(name, argT, ret) { if (argT.getArity() != 3) throw argN_oper_error(); }

ArgForm Map::parseForm(std::string symDefStr) {
    ArgForm flags;
    std::string errorMess = "Неправильно задана форма ";
    if (symDefStr.empty())
        throw std::invalid_argument(errorMess + "символа (пустая).");
    errorMess += "(\"" + symDefStr + "\") символа. ";

    flags.prefix = (symDefStr.front() == '{');
    if (flags.prefix) {
        if (symDefStr.length() < 2 || symDefStr.at(1) != '}')
            throw std::invalid_argument(errorMess + "Нет парной скобки префиксной позиции.");
        symDefStr.erase(0, 2);
    }
    if (symDefStr.front() == '{')
        throw std::invalid_argument(errorMess + "Больше одного префиксного аргумента.");

    flags.tail = 0;
    while (symDefStr.back() == '}') {
        if (symDefStr.length() < 2 || symDefStr.at(symDefStr.length()-2) != '{')
            throw std::invalid_argument(errorMess + "Нет парной скобки постфиксной позиции.");
        symDefStr.erase(symDefStr.length()-2);
        flags.tail++;
    }

    if (symDefStr.empty())
        throw std::invalid_argument(errorMess + "Пустое имя символа.");

    auto lastChar = symDefStr.back();
    flags.top    = (lastChar == '^');
    flags.bottom = (lastChar == '_');
    if (symDefStr.length() >= 2) {
        auto lastTwoChars = symDefStr.substr(symDefStr.length() - 2);
        if (lastTwoChars == "_^" || lastTwoChars == "^_")
            flags.top = flags.bottom = true;
    }
    return flags;
}

Map* Map::create(std::string symForm, const ProductMT& argT, const MathType* retT) {
    ArgForm af = parseForm(symForm);
    size_t totalArgs = af.prefix + af.top + af.bottom + af.tail;
    switch (totalArgs) {
        case 1 : {
            UnaryOperation::Form uForm;
            if (af.prefix)
                uForm = UnaryOperation::Form::PRE;
            else if (af.top)
                uForm = UnaryOperation::Form::TOP;
            else if (af.bottom)
                uForm = UnaryOperation::Form::BOT;
            else // должен быть af.tail == 1
                uForm = UnaryOperation::Form::POST;
            return new UnaryOperation(symForm, uForm, argT, retT);
        }
        case 2 : {
            BinaryOperation::Form bForm;
            if (af.tail == 2)
                bForm = BinaryOperation::Form::FOLLOW;
            else if (af.tail == 1) {
                if (af.bottom)
                    bForm = BinaryOperation::Form::BOT_MID;
                else if (af.top)
                    bForm = BinaryOperation::Form::TOP_MID;
                else // должен быть af.prefix
                    bForm = BinaryOperation::Form::INFIX;
            }
            else {
                if (af.bottom && af.top)
                    bForm = BinaryOperation::Form::BOT_TOP;
                else if (af.prefix && af.top)
                    bForm = BinaryOperation::Form::PREF_TOP;
                else // должен быть af.prefix && af.bottom
                    bForm = BinaryOperation::Form::PREF_BOT;
            }
            return new BinaryOperation(symForm, bForm, argT, retT);
        }
        case 3 : {
            if (af.top && af.bottom && af.tail == 1)
                return new TernaryOperation(symForm, argT, retT);
            else
                throw std::invalid_argument(
                    "Заданная форма записи аргументов тернарной операции не поддерживается.");
        }
        default:
            return new Map(symForm, argT, retT);
    }
}
