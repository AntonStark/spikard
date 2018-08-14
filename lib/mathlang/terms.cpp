//
// Created by anton on 15.12.16.
//

#include "terms.hpp"

Map::Map(std::string name, MTVector argT, const MathType* retT)
    : NamedTerm(std::move(name)), _type(argT, retT) {
    if (argT.size() == 0)
        throw std::invalid_argument("Нуль-арные отображения запрещены. Используйте константы.");
}

bool Map::operator<(const Map& other) const {
    if (_type != other._type)
        return (_type < other._type);
    else
        return (this->NamedTerm::operator<)(other);
}

class Map::argN_oper_error : public std::invalid_argument
{
public:
    argN_oper_error()
        : std::invalid_argument("Кол-во типов аргументов не соответствует типу операции.") {}
};

UnaryOperation::UnaryOperation(std::string name, Form form,
                               MTVector argT, const MathType* retT)
    : Map(name, argT, retT), _form(form) { if (argT.size() != 1) throw argN_oper_error(); }

BinaryOperation::BinaryOperation(std::string name, Form form,
                                 MTVector argT, const MathType* ret)
    : Map(name, argT, ret), _form(form) { if (argT.size() != 2) throw argN_oper_error(); }

TernaryOperation::TernaryOperation(std::string name, MTVector argT, const MathType* ret)
    : Map(name, argT, ret) { if (argT.size() != 3) throw argN_oper_error(); }

ArgForm Map::parseForm(std::string symDefStr) {
    ArgForm flags;
    std::string errorMess = "Неправильно задана форма ";
    if (symDefStr.empty())
        throw std::invalid_argument(errorMess + "символа (пустая).");
    errorMess += "(\"" + symDefStr + "\") символа. ";

    flags.prefix = (symDefStr.front() == '{');
    if (flags.prefix) {
        if (symDefStr.length() < 2 || symDefStr.at(1) != '}')
            throw std::invalid_argument(errorMess + "Нет парной скобки префиксной позиции.");
        symDefStr.erase(0, 2);
    }
    if (symDefStr.front() == '{')
        throw std::invalid_argument(errorMess + "Больше одного префиксного аргумента.");

    flags.tail = 0;
    while (symDefStr.back() == '}') {
        if (symDefStr.length() < 2 || symDefStr.at(symDefStr.length()-2) != '{')
            throw std::invalid_argument(errorMess + "Нет парной скобки постфиксной позиции.");
        symDefStr.erase(symDefStr.length()-2);
        flags.tail++;
    }

    if (symDefStr.empty())
        throw std::invalid_argument(errorMess + "Пустое имя символа.");

    auto lastChar = symDefStr.back();
    flags.top    = (lastChar == '^');
    flags.bottom = (lastChar == '_');
    if (symDefStr.length() >= 2) {
        auto lastTwoChars = symDefStr.substr(symDefStr.length() - 2);
        if (lastTwoChars == "_^" || lastTwoChars == "^_")
            flags.top = flags.bottom = true;
    }
    return flags;
}

std::string Map::extractName(std::string symDefStr) {
    if (symDefStr.empty())
        throw std::invalid_argument("Неправильно задана форма символа (пустая).");
    if (symDefStr.front() == '{') {
        if (symDefStr.length() < 2 || symDefStr.at(1) != '}')
            throw std::invalid_argument("Неправильно задана форма (\"" + symDefStr + "\") символа. "
                                        "Нет парной скобки префиксной позиции.");
        symDefStr.erase(0, 2);
    }

    if (symDefStr.empty())
        throw std::invalid_argument("Неправильно задана форма символа (пустая).");
    while (symDefStr.back() == '}') {
        if (symDefStr.length() < 2 || symDefStr.at(symDefStr.length()-2) != '{')
            throw std::invalid_argument("Неправильно задана форма (\"" + symDefStr + "\") символа. "
                                        "Нет парной скобки постфиксной позиции.");
        symDefStr.erase(symDefStr.length()-2);
    }

    if (symDefStr.empty())
        throw std::invalid_argument("Неправильно задана форма символа (пустая).");
    if (symDefStr.back() == '^' || symDefStr.back() == '_') {
        auto last = symDefStr.back();
        symDefStr.pop_back();
        if (symDefStr.empty())
            throw std::invalid_argument("Неправильно задана форма символа (пустая).");
        auto pred = symDefStr.back();
        if ((pred == '^' || pred == '_') && pred != last)
            symDefStr.pop_back();
    }

    if (symDefStr.empty())
        throw std::invalid_argument("Неправильно задана форма символа (пустая).");
    return symDefStr;
}

Map* Map::create(std::string symForm, MTVector argT, const MathType* retT) {
    std::string name = extractName(symForm);
    ArgForm af = parseForm(symForm);
    size_t totalArgs = af.prefix + af.top + af.bottom + af.tail;
    switch (totalArgs) {
        case 1 : {
            UnaryOperation::Form uForm;
            if (af.prefix)
                uForm = UnaryOperation::Form::PRE;
            else if (af.top)
                uForm = UnaryOperation::Form::TOP;
            else if (af.bottom)
                uForm = UnaryOperation::Form::BOT;
            else // должен быть af.tail == 1
                uForm = UnaryOperation::Form::POST;
            return new UnaryOperation(name, uForm, argT, retT);
        }
        case 2 : {
            BinaryOperation::Form bForm;
            if (af.tail == 2)
                bForm = BinaryOperation::Form::FOLLOW;
            else if (af.tail == 1) {
                if (af.bottom)
                    bForm = BinaryOperation::Form::BOT_MID;
                else if (af.top)
                    bForm = BinaryOperation::Form::TOP_MID;
                else // должен быть af.prefix
                    bForm = BinaryOperation::Form::INFIX;
            }
            else {
                if (af.bottom && af.top)
                    bForm = BinaryOperation::Form::BOT_TOP;
                else if (af.prefix && af.top)
                    bForm = BinaryOperation::Form::PREF_TOP;
                else // должен быть af.prefix && af.bottom
                    bForm = BinaryOperation::Form::PREF_BOT;
            }
            return new BinaryOperation(name, bForm, argT, retT);
        }
        case 3 : {
            if (af.top && af.bottom && af.tail == 1)
                return new TernaryOperation(name, argT, retT);
            else
                throw std::invalid_argument(
                    "Заданная форма записи аргументов тернарной операции не поддерживается.");
        }
        default:
            return new Map(name, argT, retT);
    }
}

std::string ParenSymbol::print() const {
    std::stringstream buf;
    buf << '(';
    if (!_args.empty()) {
        auto lit = _args.begin(), le = _args.end();
        buf << (*lit)->print();
        while (++lit != le)
            buf << ", " << (*lit)->print();
    }
    buf << ')';
    return buf.str();
}
std::string Term::print() const {
    std::stringstream buf;
    if (getArity() != 2)
        buf << _f.print() << ParenSymbol::print();
    else {
        buf << '(' << arg(1)->print();
        buf << _f.print();
        buf << arg(2)->print() << ')';
    }
    return buf.str();
}
std::string Term::printQ() const {
    std::stringstream buf;
    buf << '(' << _f.print();
    // по пострению arg(1) действительно Variable
    auto var = static_cast<const Variable*>(arg(1));
    buf << var->getName() << "\\in " << var->getType()->getName();
    buf << arg(2)->print() << ')';
    return buf.str();
}


class ParenSymbol::argN_argType_error : public std::invalid_argument
{
public:
    argN_argType_error()
            : std::invalid_argument("Кол-во или тип аргументов не "
                                        "соответствует символу.\n") {}
};

void ParenSymbol::checkArgs(const Map& f, TermsVector args) const {
    Map::MTVector argsTypes;
    for (const auto& a : args)
        argsTypes.push_back(a->getType());
    if (!f.matchArgType(argsTypes))
        throw argN_argType_error();
}

ParenSymbol::ParenSymbol(const TermsVector& args) {
    for (auto& a : args)
        _args.push_back(a->clone());
}

ParenSymbol::ParenSymbol(const ParenSymbol& one)
        /*: vars(one.vars)*/ {
    for (const auto& a : one._args)
        _args.push_back(a->clone());
}

ParenSymbol::~ParenSymbol() {
    for (auto a : _args)
        delete a;
}

bool ParenSymbol::operator==(const ParenSymbol& other) const {
    if (other._args.size() != _args.size())
        return false;
    for (size_t i = 0; i < _args.size(); ++i)
        if (!_args[i]->comp(other._args[i]))
            return false;
    return true;
}

bool NamedTerm::comp(const Terms* other) const {
    if (auto namedOther = dynamic_cast<const NamedTerm*>(other))
        return (*getType() == *namedOther->getType()
                && getName() == namedOther->getName());
    else
        return false;
}
bool Term::comp(const Terms* other) const {
    if (auto t = dynamic_cast<const Term*>(other))
        return (_f == t->_f && ParenSymbol::operator==(*t));
    else
        return false;
}

const Terms* NamedTerm::get(Path path) const
{ return (path.empty() ? this : nullptr); }
const Terms* Term::get(Path path) const {
    if (path.empty())
        return this;
    else {
        auto p = path.top(); path.pop();
        return (p > getArity() ? nullptr : arg(p)->get(path));
    }
}

Terms* NamedTerm::replace(Path path, const Terms* by) const
{ return (path.empty() ? by->clone() : nullptr); }
const ParenSymbol::TermsVector&
ParenSymbol::replace(Terms::Path path, const Terms* by) const {
    TermsVector updated;
    auto p = path.top();
    path.pop();
    for (unsigned i = 0; i < _args.size(); ++i)
        if (i == p-1)
            updated.push_back(_args[i]->replace(path, by));
        else
            updated.push_back(_args[i]->clone());
    return updated;
}
Terms* Term::replace(Path path, const Terms* by) const {
    if (path.empty())
        return by->clone();
    if (path.top() > getArity())
        return nullptr;
    return new Term(_f, ParenSymbol::replace(path, by));
}

Term::Term(Map f, ParenSymbol::TermsVector args)
        : ParenSymbol(args), _f(f) {
    checkArgs(f, args);

    for (const auto& a : args) {
        if (auto var = dynamic_cast<const Variable*>(a))
            free.insert(*var);
        else if (auto term = dynamic_cast<const Term*>(a)) {
            for (const auto& v : term->free)
                free.insert(v);
        }
        // else a  - это Constant - пропускаем
    }
}

Map takeFirstMatchTypes(std::set<Map> symSet,
                           const ParenSymbol::TermsVector& args) {
    std::vector<const MathType*> argsType;
    for (const auto& a : args)
        argsType.push_back(a->getType());

    for (const auto& s : symSet)
        if (s.matchArgType(argsType))
            return s;
    throw ParenSymbol::argN_argType_error();
}
Term::Term(std::set<Map> symSet, TermsVector args)
    : Term(takeFirstMatchTypes(std::move(symSet), args), args) {}

void Term::boundVar(Variable var) {
    auto search = free.find(var);
    if (search != free.end())
        free.erase(search);
    else
        throw std::invalid_argument("Попытка ограничения не "
                                        "свободной перменной.\n");
}

std::map<Term::QType, const std::string>
        Term::qword = { {Term::QType::FORALL,"\\forall "},
                        {Term::QType::EXISTS,"\\exists "} };
Map forall(Term::qword[Term::QType::FORALL],
              {&any_mt, &logical_mt}, &logical_mt);
Map exists(Term::qword[Term::QType::EXISTS],
              {&any_mt, &logical_mt}, &logical_mt);

Terms* NamedTerm::replace(const Terms* x, const Terms* t) const
{ return (comp(x) ? t->clone() : this->clone()); }

Terms* Term::replace(const Terms* x, const Terms* t) const {
    // если x переменная...
    if (auto var = dynamic_cast<const Variable*>(x)) {
        // ... а наш терм от неё не звисит
        if (free.find(*var) == free.end())
            return this->clone();
    }
    else if (comp(x))
        return t->clone();

    TermsVector args;
    for (auto& a : _args)
        args.push_back(a->replace(x, t));
    return new Term(_f, args);
}

Terms* ForallTerm::replace(const Terms* x, const Terms* t) const {
    // если x переменная...
    if (auto var = dynamic_cast<const Variable*>(x)) {
        // ... а наш терм от неё не звисит
        if (free.find(*var) == free.end())
            return this->clone();
    }
    else if (comp(x))
        return t->clone();

    Terms* termReplaced = arg(2)->replace(x, t);
    return new ForallTerm(
        static_cast<Variable*>(arg(1)->clone()),
        termReplaced);
}

Terms* ExistsTerm::replace(const Terms* x, const Terms* t) const {
    // если x переменная...
    if (auto var = dynamic_cast<const Variable*>(x)) {
        // ... а наш терм от неё не звисит
        if (free.find(*var) == free.end())
            return this->clone();
    }
    else if (comp(x))
        return t->clone();

    Terms* termReplaced = arg(2)->replace(x, t);
    return new ExistsTerm(
        *static_cast<Variable*>(arg(1)->clone()),
        termReplaced);
}
