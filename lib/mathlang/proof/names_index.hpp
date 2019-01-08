//
// Created by anton on 19.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMES_HPP
#define SPIKARD_MATHLANG_NAMES_HPP

#include <string>
#include <map>
#include <set>

//#include "../basics/namestype.hpp"
#include "../parser/lexeme.hpp"
#include "../parser/lexer.hpp"

class Definition;
class NameSpaceIndex
/// Инкапсулирует работу с именами: имя-сущность, уникальность, доступ
{
public:
    typedef Parser2::LexemeSequence NamesType; // todo вынести в файл lexer_basic (как назвать?) и включать его, а не lexer.hpp
private:
    std::map<NamesType, Definition*> data;

    class name_doubling;
    class no_name;
public:
    void add(const NamesType& name, Definition* where);
    bool isThatName(const NamesType& name) const;

    std::vector<NamesType> getNames() const;
    std::set<std::string> getNamesStr() const;
    Definition* get(const NamesType& name) const;
};

class NameStoringStrategy
/// Интерфейс работы с именами со стороны узлов
{
public:
    virtual const NameSpaceIndex& index() const = 0;
    friend class Definition;
    virtual void registerName(const Parser2::LexemeSequence& name, Definition* where) = 0;

    virtual std::string printType() const = 0;
};

#endif //SPIKARD_MATHLANG_NAMES_HPP
