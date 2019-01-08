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

#include "../basics/mapterm.hpp"
#include "../basics/mathtype.hpp"
#include "../basics/terms.hpp"

#include "../parser/lexer.hpp"

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
/// Именно в этом классе может (и, похоже, семантически должен) происходить
/// перевод строки (от клиента) в LexemeSequence для NSI
{
public:
    enum class NameTy {SYM, VAR, MT};
private:
    PrimaryMT* mtype; // fixme пока используется явный union
    NamedTerm* term;

    friend class PrimaryNode;
    static Hierarchy* fromJson(const json& j, Node* parent, NameTy type);
public:
    const NameTy defType;
    ~Definition() override = default;
    Definition(const Definition&) = delete;
    Definition& operator=(const Definition&) = delete;
    // todo посмотреть может после редукции NameTy разные конструкторы можно объединить
    Definition(Node* parent, const std::string& typeName)
        : Item(parent), defType(NameTy::MT) {
        Parser2::CurAnalysisData cad = Parser2::texLexer.recognize(typeName);
        if (!cad.res.success)
            throw Parser2::parse_error(cad.res);
        if (cad.blankFound)
            throw Parser2::parse_error("имя типа и переменной не может содержать команд отступа.");

        mtype = new PrimaryMT(typeName);

        parent->registerName(cad.filtered, this);
    }
    Definition(Node* parent, const std::string& name, const MathType* mathType)
        : Item(parent), defType(NameTy::VAR) {
        Parser2::CurAnalysisData cad = Parser2::texLexer.recognize(name);
        if (!cad.res.success)
            throw Parser2::parse_error(cad.res);
        if (cad.blankFound)
            throw Parser2::parse_error("имя типа и переменной не может содержать команд отступа.");

        term = new Variable(cad.filtered, mathType);
        parent->registerName(cad.filtered, this);
    }
    // вместо symName (напр. \Rightarrow ) теперь symForm (напр. {}\Rightarrow{} или другое обозначение инфиксности)
    // ещё примеры \sum_^{} \frac{}{} {}+{} A_{} (как  A_i v)
    // пустой формат (symForm = f) соответствует функциональной форме записи аргументов (арность брать из описания сигнатуры)
    // в то же время синтаксис задания множества можно описать как: \{{}\in {} | {}\}
    // где три аргументных места и ProductMT argT = any x Set x (any -> Logical)
    Definition(Node* parent, const std::string& symForm,
        const ProductMT& argT, const MathType* retT)
        : Item(parent), defType(NameTy::SYM) {
        Parser2::CurAnalysisData cad = Parser2::texLexer.recognize(symForm);
        if (!cad.res.success)
            throw Parser2::parse_error(cad.res);

//        term = Map::create(symForm, argT, retT);
        term = new Map(cad.filtered, argT, retT);
        // todo подумать как быть с blank в символе. просто удалять? (позже, пока считаем, что blank cmd нет)
        /// ответ: проводим восстанавливающее преобразование (описывается в лексере, после ввода Lexeme::originOffset)
        /// [originOffset нужно для сообщения об ошибках в исходной строке]
        /// нужно выделить аргументные места (или их лексер распознает соотв. лексемами?)
        parent->registerName(cad.filtered, this);
    }

    std::string print(Representation* r, bool incremental) const override
    { r->process(this); return r->str(); }
    PrimaryMT* getType() const { return mtype; }
    NamedTerm* getTerm() const { return term; }
    Map* getMap() const { return dynamic_cast<Map*>(term); }
};

#endif //SPIKARD_MATHLANG_DEFINITION_HPP
