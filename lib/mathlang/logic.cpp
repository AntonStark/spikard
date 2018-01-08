//
// Created by anton on 15.12.16.
//

#include "logic.hpp"

bool Named::operator==(const Named& one) const
{ return (_name == one._name); }
bool MathType::operator==(const MathType& one) const {
    return (getName() == "any" || one.getName() == "any"
            || (this->Named::operator==)(one));
}
bool Map::operator==(const Map& one) const
{ return (_argT == one._argT && _retT == one._retT); }
bool Symbol::operator==(const Symbol& one) const {
    return ( (this->Named::operator==)(one)
             && (this->Map::operator==)(one) ); }

bool Named::operator<(const Named& other) const
{ return (_name < other._name); }
bool MathType::operator<(const MathType& other) const
{ return (this->Named::operator<)(other); }
bool Map::operator<(const Map& other) const {
    if (_argT < other._argT) return true;
    else if (other._argT < _argT) return false;
    else return (_retT < other._retT);
}
bool Symbol::operator<(const Symbol& other) const {
    if ((this->Map::operator<)(other))
        return true;
    else if (other.Map::operator<(*this))
        return false;
    else
        return (this->Named::operator<)(other);
}

MathType logical_mt("Logical");

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
std::string Variable::print() const
{ return getName(); }
std::string Term::print() const {
    std::stringstream buf;
    if (getArity() != 2)
        buf << Symbol::print() << ParenSymbol::print();
    else {
        buf << '(' << arg(1)->print();
        buf << Symbol::print();
        buf << arg(2)->print() << ')';
    }
    return buf.str();
}
std::string Term::printQ() const {
    std::stringstream buf;
    buf << '(' << Symbol::print();
    // по пострению arg(1) действительно Variable
    auto var = static_cast<const Variable*>(arg(1));
    buf << var->getName() << "\\in " << var->getType().getName();
    buf << arg(2)->print() << ')';
    return buf.str();
}


class ParenSymbol::argN_argType_error: public std::invalid_argument
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
        if (!_args[i]->doCompare(other._args[i]))
            return false;
    return true;
}

bool Variable::doCompare(const Terms* other) const
{
    if (const Variable* v = dynamic_cast<const Variable*>(other))
        return (this->Named::operator==)(*v);
    else
        return false;
}

Terms* Variable::replace(const Terms* x, const Terms* t) const
{
    if (doCompare(x))
        return t->clone();
    else
        return this->clone();
}

Terms* Variable::replace(Path where, const Terms* by) const
{
    if (where.size() == 0)
        return by->clone();
    else
        return nullptr;
}

const Terms* Variable::get(Path path) const
{
    if (path.size() == 0)
        return this;
    else
        return nullptr;
}

Term::Term(Symbol f, std::vector<Terms*> args)
        : Terms(f.getType()), Symbol(f), ParenSymbol(args)
{
    checkArgs(f, args);

    for (auto& a : args)
    {
        if (Variable* var = dynamic_cast<Variable*>(a))
            free.insert(*var);
        else if (Term* term = dynamic_cast<Term*>(a))
            for (auto& v : term->free)
                free.insert(v);
    }
}

Symbol takeFirstMatchTypes(std::set<Symbol> syms,
                           const std::vector<Terms*>& args) {
    std::vector<MathType> argsType;
    for (const auto& a : args)
        argsType.push_back(a->getType());

    for (const auto& s : syms)
        if (s.matchArgType(argsType))
            return s;
    throw ParenSymbol::argN_argType_error();
}
Term::Term(std::set<Symbol> fset, std::vector<Terms*> _args)
    : Term(takeFirstMatchTypes(fset, _args), _args) {}

void Term::boundVar(Variable var)
{
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
Symbol forall(Term::qword[Term::QType::FORALL],
              {MathType("any"), logical_mt}, logical_mt);
Symbol exists(Term::qword[Term::QType::EXISTS],
              {MathType("any"), logical_mt}, logical_mt);

bool Term::doCompare(const Terms* other) const
{
    if (const Term* t = dynamic_cast<const Term*>(other))
        return ( (this->Symbol::operator==)(*t)
                 && (this->ParenSymbol::operator==)(*t));
    else
        return false;
}

Terms* Term::replace(const Terms* x, const Terms* t) const {
    // если x переменная...
    if (auto var = dynamic_cast<const Variable*>(x)) {
        // ... а наш терм от неё не звисит
        if (free.find(*var) == free.end())
            return this->clone();
    }
    else {
        if (doCompare(x))
            return t->clone();
    }

    std::vector<Terms*> args;
    for (auto& a : _args)
        args.push_back(a->replace(x, t));
    return new Term(*this, args);
}

Terms* Term::replace(Path where, const Terms* by) const
{
    if (where.size() == 0)
        return by->clone();
    else
    {
        auto n = where.front();
        where.pop_front();
        if (n > _args.size())
            return nullptr;
        std::vector<Terms*> _args;
        for (unsigned i = 0; i < _args.size(); ++i)
            if (i != n-1)
                _args.push_back(_args[i]->clone());
            else
                _args.push_back(_args[i]->replace(where, by));
        return (new Term(*this, _args));
    }
}

const Terms* Term::get(Path path) const
{
    if (path.size() == 0)
        return this;
    else
    {
        auto n = path.front();
        path.pop_front();
        if (n > getArity())
            return nullptr;
        else
            return arg(n)->get(path);
    }
}

Terms* ForallTerm::replace(const Terms* x, const Terms* t) const {
    // если x переменная...
    if (auto var = dynamic_cast<const Variable*>(x)) {
        // ... а наш терм от неё не звисит
        if (free.find(*var) == free.end())
            return this->clone();
    }
    else {
        if (doCompare(x))
            return t->clone();
    }

    Terms* termReplaced = arg(2)->replace(x, t);
    return new ForallTerm(
        *static_cast<Variable*>(arg(1)->clone()),
        termReplaced);
}

Terms* ExistsTerm::replace(const Terms* x, const Terms* t) const {
    // если x переменная...
    if (auto var = dynamic_cast<const Variable*>(x)) {
        // ... а наш терм от неё не звисит
        if (free.find(*var) == free.end())
            return this->clone();
    }
    else {
        if (doCompare(x))
            return t->clone();
    }

    Terms* termReplaced = arg(2)->replace(x, t);
    return new ExistsTerm(
        *static_cast<Variable*>(arg(1)->clone()),
        termReplaced);
}
