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
#include "../parser/lexer.hpp"

using json = nlohmann::json;

PrimaryMT* getType (const NameSpaceIndex& index, const std::string& name);
NamedTerm* getVar  (const NameSpaceIndex& index, const std::string& name);
NamedTerm* getConst(const NameSpaceIndex& index, const std::string& name);
NamedTerm* getSym  (const NameSpaceIndex& index, const std::string& name);

struct parse_error : public std::invalid_argument {
    parse_error(const Parser2::ParseStatus& errRes)
        : std::invalid_argument("Ошибка на " + std::to_string(errRes.at) + " символе: " + errRes.mess) {}
    parse_error(const std::string& mess)
        : std::invalid_argument("Ошибка: " + mess) {}
};

class Definition : public Item
/// Это базовый класс определений.
/// Отвечает за регистрацию (тип, имя, опр-е) в Namespace и хранение терма
/// Именно в этом классе может (и, похоже, семантически должен) происходить
/// перевод строки (от клиента) в LexemeSequence для NSI
{
private:
    PrimaryMT* mtype; // fixme пока используется явный union
    NamedTerm* term;
    Definition(Node* parent, NameTy type, const std::string& name)
            : Item(parent), defType(type) {
        Parser2::CurAnalysisData cad = Parser2::texLexer.recognize(name);
        if (!cad.res.success)
            throw parse_error(cad.res);
        if (type == NameTy::SYM) {
            // todo подумать как быть с blank в символе. просто удалять? (позже, пока считаем, что blank cmd нет)
            /// ответ: проводим восстанавливающее преобразование (описывается в лексере, после ввода Lexeme::originOffset)
            /// нужно выделить аргументные места (или их лексер распознает соотв. лексемами?)
        }
        else if (cad.blankFound)
            throw parse_error("имя типа и переменной не может содержать команд отступа.");
        parent->registerName(type, cad.lexems, this);
    }

    friend class PrimaryNode;
    static Hierarchy* fromJson(const json& j, Node* parent, NameTy type);
public:
    const NameTy defType;
    ~Definition() override = default;
    Definition(const Definition&) = delete;
    Definition& operator=(const Definition&) = delete;

    Definition(Node* parent, const std::string& typeName)
        : Definition(parent, NameTy::MT, typeName)
        { mtype = new PrimaryMT(typeName); }
    // todo подумать как можно лучше увязать создание переменных и констант
    /**
     * Возможно следует вообще упразднить класс констант, поскольку есть неполные фиксации
     */
    Definition(Node* parent, NameTy type,
        const std::string& name, const MathType* mathType)
        : Definition(parent, type, name) {
        if (type == NameTy::VAR)
            term = new Variable(name, mathType);
        else if (type == NameTy::CONST)
            term = new Constant(name, mathType);
        else {}
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
