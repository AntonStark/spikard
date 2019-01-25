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
#include "../basics/mapterm.hpp"
#include "../basics/mathtype.hpp"
#include "../basics/texname.hpp"

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
    virtual NamedTerm* _get() = 0;
public:
    virtual NamedTerm* use(Item* in) = 0;
};

class DefType : public Item, public Definition
{
private:
    PrimaryMT* type;
    DefType(Node* parent, const std::string& typeName);
protected:
    PrimaryMT* _get() override
    { return type; }
public:
    ~DefType() override;
    static DefType* create(Node* parent, const std::string& typeName)
    { return new DefType(parent, typeName); }

    PrimaryMT* use(Item* in) override;
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
    Map* funct;
    DefFunct(Node* parent, const std::string& fName,
             DefType* argT, DefType* retT);
protected:
    Map* _get() override
    { return funct; }
public:
    ~DefFunct() override;
    static DefFunct* create(Node* parent, const std::string& symForm,
        const std::vector<DefType*>& argT, DefType* retT)
    { return new DefFunct(parent, symForm, argT, retT); }

    Map* use(Item* in) override;
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
    AbstractConnective* connective;
    DefConnective(Node* parent, const std::string& sym, bool prefix,
                  DefType* argT, DefType* retT);
    DefConnective(Node* parent, const std::string& sym, BinaryOperation::Notation notation,
                  DefType* leftT, DefType* rightT, DefType* retT);
    /*
     * для особых связок, после todo SpecialConnective
    DefConnective(Node* parent, const std::string& form,
        const std::vector<DefType*>& argT, DefType* retT)
    : Item(parent) {
        MathType::MTVector argTypes;
        for (auto* d : argT)
            argTypes.push_back(d->use(this));
        auto argType = ProductMT(argTypes);

        auto retType = retT->use(this);
        auto* name = new TexName(form);
        connective = new Map(name, argType, retType);
    }*/
public:
    ~DefConnective() override;
    static DefConnective* create(Node* parent, const std::string& sym, bool prefix,
                                 DefType* argT, DefType* retT)
    { return new DefConnective(parent, sym, prefix, argT, retT); }
    static DefConnective* create(Node* parent, const std::string& sym, BinaryOperation::Notation notation,
                                 DefType* leftT, DefType* rightT, DefType* retT)
    { return new DefConnective(parent, sym, notation, leftT, rightT, retT); }
    std::string print(Representation* r, bool incremental) const override;
};

#endif //SPIKARD_MATHLANG_DEFINITION_HPP
