//
// Created by anton on 19.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMES_HPP
#define SPIKARD_MATHLANG_NAMES_HPP

#include <string>
#include <map>
#include <set>

#include "../basics/primary.hpp"

class Definition;
class AbstractName;
class MathType;

/**
 * @brief Отвечает за эффективное хранение информации о именах и их типах
 *
 * От хранилища понадобиться только два действия: вывести все имена данного типа и указать определение имени.
 * Храним имена и определения в векторах. Также храним соответствие тип -> набор смещений и имя -> смещение.
 */
class NameSpaceIndex
{
private:
    std::vector<const AbstractName*> names;
    std::vector<Definition*> definitions;

    std::map<std::string, std::vector<size_t> > type2IDs;
    std::map<std::string, size_t> name2ID;

    class name_doubling;
    class no_name;

    bool exists(const std::string& str) const;
public:
    void add(const NamedEntity* named, const MathType* type, Definition* where);

    std::vector<const AbstractName*> getNames(const MathType* type = nullptr) const;
    std::vector<std::string> getNamesStr(const MathType* type = nullptr) const;
    Definition* get(const AbstractName* name) const;
};

class NameStoringStrategy
/// Интерфейс работы с именами со стороны узлов
{
public:
    enum class BasicNSSTypes {Appending, Hidden};
    virtual const NameSpaceIndex& index() const = 0;
    friend class Definition;
    virtual void registerNamed(const NamedEntity* named, const MathType* type, Definition* where) = 0;

    virtual std::string printType() const = 0;
};

#endif //SPIKARD_MATHLANG_NAMES_HPP
