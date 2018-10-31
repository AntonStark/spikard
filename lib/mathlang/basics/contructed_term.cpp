//
// Created by anton on 28.09.18.
//

#include "contructed_term.hpp"

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

bool Term::comp(const Terms* other) const {
    if (auto t = dynamic_cast<const Term*>(other))
        return (_f == t->_f && ParenSymbol::operator==(*t));
    else
        return false;
}

const Terms* Term::get(Path path) const {
    if (path.empty())
        return this;
    else {
        auto p = path.top(); path.pop();
        return (p > getArity() ? nullptr : arg(p)->get(path));
    }
}

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
           ProductMT({&any_mt, &logical_mt}), &logical_mt);
Map exists(Term::qword[Term::QType::EXISTS],
           ProductMT({&any_mt, &logical_mt}), &logical_mt);


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
        static_cast<Variable*>(arg(1)->clone()),
        termReplaced);
}
