//
// Created by anton on 19.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMES_HPP
#define SPIKARD_MATHLANG_NAMES_HPP

#include <string>
#include <map>
#include <set>

#include "../parser/lexeme.hpp"
#include "../parser/lexer.hpp"

class Definition;
class NameSpaceIndex
/// Инкапсулирует работу с именами: имя-сущность, уникальность, доступ
{
public:
    typedef Parser2::LexemeSequence NamesType; // todo вынести в файл lexer_basic (как назвать?) и включать его, а не lexer.hpp
    enum class NameTy {SYM, CONST, VAR, MT};
private:
    std::map<NamesType, std::pair<NameTy, Definition*> > data;

    class name_doubling;
    class no_name;
public:
    void add(NameTy type, const NamesType& name, Definition* where);
    bool isThatType(const NamesType& name, const NameTy& type) const;
    bool isSomeType(const NamesType& name) const;

    std::set<NamesType> getNames(NameTy type) const;
    std::set<std::string> getNamesStr(NameTy type) const;
    Definition* get(NameTy type, const NamesType& name) const;
};
typedef NameSpaceIndex::NameTy NameTy;

class NameStoringStrategy
/// Интерфейс работы с именами со стороны узлов
{
public:
    virtual const NameSpaceIndex& index() const = 0;
    friend class Definition;
    virtual void registerName(
        NameTy type, const Parser2::LexemeSequence& name, Definition* where) = 0;

    virtual std::string printType() const = 0;
};

#endif //SPIKARD_MATHLANG_NAMES_HPP
