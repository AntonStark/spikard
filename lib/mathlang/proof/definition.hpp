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

#include "../consepts/terms.hpp"

#include "../basics/mapterm.hpp"
#include "../basics/mathtype.hpp"
#include "../basics/texname.hpp"

#include "../parser/lexer.hpp"

#include "names_index.hpp"
#include "structure.hpp"

using json = nlohmann::json;

PrimaryMT* getType(const NameSpaceIndex& index, const std::string& name);

class Definition : public Item
/// Это базовый класс определений.
/// Отвечает за регистрацию (тип, имя, опр-е) в Namespace и хранение терма
/// Именно в этом классе может (и, похоже, семантически должен) происходить
/// перевод строки (от клиента) в LexemeSequence для NSI
{
public:
    enum class NameTy {SYM, VAR, MT};
private:
    NamedTerm* term;
    std::set<Item*> _use;

//    friend class PrimaryNode;
//    static Hierarchy* fromJson(const json& j, Node* parent, NameTy type);
    // todo посмотреть может после редукции NameTy разные конструкторы можно объединить и нужен ли defType
    Definition(Node* parent, const std::string& typeName)
        : Item(parent), defType(NameTy::MT) {
        auto* name = new TexName(typeName, true);
        term = new PrimaryMT(name);
        parent->registerNamedTerm(term, this);
    }
    Definition(Node* parent, const std::string& varName, Definition* mathType)
        : Item(parent), defType(NameTy::VAR) {
        auto* name = new TexName(varName, true);
        auto* type = dynamic_cast<MathType*>(mathType->use(this));
        term = new Variable(name, type);
        parent->registerNamedTerm(term, this);
    }
    // вместо symName (напр. \Rightarrow ) теперь symForm (напр. {}\Rightarrow{} или другое обозначение инфиксности)
    // ещё примеры \sum_^{} \frac{}{} {}+{} A_{} (как  A_i v)
    // пустой формат (symForm = f) соответствует функциональной форме записи аргументов (арность брать из описания сигнатуры)
    // в то же время синтаксис задания множества можно описать как: \{{}\in {} | {}\}
    // где три аргументных места и ProductMT argT = any x Set x (any -> Logical)
    Definition(Node* parent, const std::string& symForm,
               const std::vector<Definition*>& argT, Definition* retT)
        : Item(parent), defType(NameTy::SYM) {
        MathType::MTVector argTypes;
        for (auto* d : argT)
            argTypes.push_back(dynamic_cast<MathType*>(d->use(this)));
        auto argType = ProductMT(argTypes);
        auto retType = dynamic_cast<MathType*>(retT->use(this));
        auto* name = new TexName(symForm);
        term = new Map(name, argType, retType);
        // todo подумать как быть с blank в символе. просто удалять? (позже, пока считаем, что blank cmd нет)
        /// ответ: проводим восстанавливающее преобразование (описывается в лексере, после ввода Lexeme::originOffset)
        /// [originOffset нужно для сообщения об ошибках в исходной строке]
        /// нужно выделить аргументные места (или их лексер распознает соотв. лексемами?)
        parent->registerNamedTerm(term, this);
    }
public:
    const NameTy defType;
    ~Definition() override = default;
    Definition(const Definition&) = delete;
    Definition& operator=(const Definition&) = delete;

    static Definition* create(Node* parent, const std::string& typeName)
    { return new Definition(parent, typeName); }
    static Definition* create(Node* parent, const std::string& varName, Definition* mathType)
    { return new Definition(parent, varName, mathType); }
    static Definition* create(Node* parent, const std::string& symForm,
                              const std::vector<Definition*>& argT, Definition* retT)
    { return new Definition(parent, symForm, argT, retT); }

    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
    PrimaryMT* getType() const { return dynamic_cast<PrimaryMT*>(term); }
    NamedTerm* getTerm() const { return term; }
    NamedTerm* use(Item* in) {
        _use.insert(in);
        return term;
    }

    Map* getMap() const { return dynamic_cast<Map*>(term); }
};

#endif //SPIKARD_MATHLANG_DEFINITION_HPP
