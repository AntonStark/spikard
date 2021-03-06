//
// Created by anton on 10.01.17.
//

#ifndef SPIKARD_MATHLANG_DEFINITION_HPP
#define SPIKARD_MATHLANG_DEFINITION_HPP

#include <string>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
#include <sstream>

#include "../../../json.hpp"

#include "../consepts/abstract_connective.hpp"
#include "../consepts/terms.hpp"

#include "../basics/function.hpp"
#include "../basics/primary.hpp"
#include "../basics/tex_name.hpp"

#include "../parser/lexer.hpp"

#include "structure.hpp"

using json = nlohmann::json;

/**
 * @brief Это абстрактный класс определений.
 *
 * Отвечает за отслеживание использований определения.
 */
class Definition
{
private:
    std::set<Item*> _use;
protected:
    void addUsage(Item* in)
    { _use.insert(in); }
    virtual NamedEntity* _get() = 0;
public:
    virtual NamedEntity* use(Item* in) = 0;
    std::set<Item*> uses() const
    { return _use; }
    NamedEntity* see()
    { return _get(); }
};

class DefType : public Item, public Definition
{
private:
    PrimaryType* type;
    DefType(Node* parent, const std::string& typeName);
protected:
    PrimaryType* _get() override
    { return type; }
public:
    ~DefType() override;
    static DefType* create(Node* parent, const std::string& typeName)
    { return new DefType(parent, typeName); }

    PrimaryType* use(Item* in) override;
    std::string print(Representation* r, bool incremental) const override;
};

class DefAtom : public Item, public Definition
{
private:
    Variable* atom;
    DefAtom(Node* parent, const std::string& varName, DefType* mathType);

protected:
    Variable* _get() override
    { return atom; }
public:
    ~DefAtom() override;
    static DefAtom* create(Node* parent, const std::string& varName, DefType* mathType)
    { return new DefAtom(parent, varName, mathType); }

    Variable* use(Item* in) override;
    std::string print(Representation* r, bool incremental) const override;
};

/**
 * @brief Класс для определений обычных функций одного аргумента.
 */
class DefFunct : public Item, public Definition
{
private:
    Function* funct;
    DefFunct(Node* parent, const std::string& fName,
             DefType* argT, DefType* retT);
protected:
    Function* _get() override
    { return funct; }
public:
    ~DefFunct() override;
    static DefFunct* create(Node* parent, const std::string& fName,
                            DefType* argT, DefType* retT)
    { return new DefFunct(parent, fName, argT, retT); }

    Function* use(Item* in) override;
    std::string print(Representation* r, bool incremental) const override;
};

/**
 * @brief Класс для определения связок: унарных и бинарных операций, а также нестандартных конструкций.
 *
 * Пока используется механизм определения формы записи через аргументные места, которые обозначаются с
 * помощью TeX-команд \cdot и \_. Последняя означает аргументное место с вводом новых имён. Например:
 *  \cdot+\cdot    A_\_\cdot    \sum_\_^\cdot\cdot    \{\_|\cdot\}
 */
class DefConnective : public Item, public Definition
{
private:
    PrintableConnective* connective;
    DefConnective(Node* parent, const std::string& sym, bool prefix,
                  DefType* argT, DefType* retT);
    DefConnective(Node* parent, const std::string& sym, BinaryOperation::Notation notation,
                  DefType* leftT, DefType* rightT, DefType* retT);
    DefConnective(Node* parent, const std::string& form,
        const std::vector<DefType*>& argT, DefType* retT);
protected:
    PrintableConnective* _get() override
    { return connective; }
public:
    ~DefConnective() override;
    static DefConnective* create(Node* parent, const std::string& sym, bool prefix,
                                 DefType* argT, DefType* retT)
    { return new DefConnective(parent, sym, prefix, argT, retT); }
    static DefConnective* create(Node* parent, const std::string& sym, BinaryOperation::Notation notation,
                                 DefType* leftT, DefType* rightT, DefType* retT)
    { return new DefConnective(parent, sym, notation, leftT, rightT, retT); }
    static DefConnective* create(Node* parent, const std::string& form,
                                 const std::vector<DefType*>& argT, DefType* retT)
    { return new DefConnective(parent, form, argT, retT); }

    NamedEntity* use(Item* in) override;
    std::string print(Representation* r, bool incremental) const override;
};

#endif //SPIKARD_MATHLANG_DEFINITION_HPP
