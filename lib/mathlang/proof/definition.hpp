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

#include "../basics/mathtype.hpp"
#include "../basics/terms.hpp"
#include "../basics/mapterm.hpp"
#include "names_index.hpp"
#include "structure.hpp"

using json = nlohmann::json;

PrimaryMT* getType (const NameSpaceIndex& index, const std::string& name);
NamedTerm* getVar  (const NameSpaceIndex& index, const std::string& name);
NamedTerm* getConst(const NameSpaceIndex& index, const std::string& name);
NamedTerm* getSym  (const NameSpaceIndex& index, const std::string& name);

class Definition : public Item
/// Это базовый класс определений.
/// Отвечает за регистрацию (тип, имя, опр-е) в Namespace и хранение терма
{
private:
    PrimaryMT* mtype; // fixme пока используется явный union
    NamedTerm* term;
    Definition(Node* parent, NameTy type, const std::string& name)
            : Item(parent), defType(type) { parent->registerName(type, name, this); }

    friend class PrimaryNode;
    static Hierarchy* fromJson(const json& j, Node* parent, NameTy type);
public:
    const NameTy defType;
    ~Definition() override = default;
    Definition(const Definition&) = delete;
    Definition& operator=(const Definition&) = delete;

    Definition(Node* parent, const std::string& typeName)
        : Definition(parent, NameTy::MT, typeName) { mtype = new PrimaryMT(typeName); }
    Definition(Node* parent, NameTy type,   // todo подумать как можно лучше увязать создание переменных и констант.
        /// Ведь по сути константа - это переменная с зафиксированным значением
        const std::string& name, const MathType* mathType)
        : Definition(parent, type, name) {
        if (type == NameTy::VAR)
            term = new Variable(name, mathType);
        else if (type == NameTy::CONST)
            term = new Constant(name, mathType);
        else {} // todo подумать[2]
    }
    // вместо symName (напр. \Rightarrow ) теперь symForm (напр. {}\Rightarrow{} или другое обозначение инфиксности)
    // ещё примеры \sum_^{} \frac{}{} {}+{} A_{} (как  A_i v)
    // пустой формат (symForm = f) соответствует функциональной форме записи аргументов (арность брать из описания сигнатуры)
    // в то же время синтаксис задания множества можно описать как: \{{}\in {} | {}\}
    // где три аргументных места и ProductMT argT = any x Set x (any -> Logical)
    Definition(Node* parent, const std::string& symForm,
        const ProductMT& argT, const MathType* retT)
        : Definition(parent, NameTy::SYM, symForm)
    { term = Map::create(symForm, argT, retT); }

    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
    PrimaryMT* getType() const { return mtype; }
    NamedTerm* getTerm() const { return term; }
    Map* getMap() const { return dynamic_cast<Map*>(term); }
};

#endif //SPIKARD_MATHLANG_DEFINITION_HPP
