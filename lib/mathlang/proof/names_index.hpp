//
// Created by anton on 19.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMES_HPP
#define SPIKARD_MATHLANG_NAMES_HPP

#include <string>
#include <map>
#include <set>

class Definition;
class NameSpaceIndex
/// Инкапсулирует работу с именами: имя-сущность, уникальность, доступ
{
public:
    enum class NameTy {SYM, CONST, VAR, MT};
private:
    std::map<std::string, std::pair<NameTy, Definition*> > data;

    class name_doubling;
    class no_name;
public:
    void add(NameTy type, const std::string& name, Definition* where);
    bool isThatType(const std::string& name, const NameTy& type) const;
    bool isSomeType(const std::string& name) const;

    std::set<std::string> getNames(NameTy type) const;
    Definition* get(NameTy type, const std::string& name) const;
};
typedef NameSpaceIndex::NameTy NameTy;

class NameStoringStrategy
/// Интерфейс работы с именами со стороны узлов
{
public:
    virtual const NameSpaceIndex& index() const = 0;
    friend class Definition;
    virtual void registerName(
        NameTy type, const std::string& name, Definition* where) = 0;

    virtual std::string printType() const = 0;
};

#endif //SPIKARD_MATHLANG_NAMES_HPP
