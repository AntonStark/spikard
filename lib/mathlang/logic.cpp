//
// Created by anton on 15.12.16.
//

#include "logic.hpp"

bool Named::operator==(const Named& one) const
{ return (_name == one._name); }
bool PrimaryMT::operator==(const MathType& one) const {
    if (getName() == "any")
        return true;
    if (one.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(one);
        return (pmt.getName() == "any" || getName() == pmt.getName());
    }
    else
        return false;
}
bool ProductMT::operator==(const MathType& one) const {
    if (one.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(one);
        return (pmt.getName() == "any");
    }
    else {
        // todo избавиться от try-catch
        try
        {
            auto& pmt = dynamic_cast<const ProductMT&>(one);
            if (_subTypes.size() != pmt._subTypes.size())
                return false;
            for (size_t i = 0; i < _subTypes.size(); ++i)
                if (*_subTypes[i] != *pmt._subTypes[i])
                    return false;
            return true;
        } catch (std::bad_cast& e)
        { return false; }
    }
}
bool MapMT::operator==(const MathType& one) const {
    if (one.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(one);
        return (pmt.getName() == "any");
    }
    else {
        try
        {
            auto& mmt = dynamic_cast<const MapMT&>(one);
            return (*_argsT == *mmt._argsT && *_retT == *mmt._retT);
        } catch (std::bad_cast& e)
        { return false; }
    }
}
bool Map::operator==(const Map& one) const
{ return (_type == one._type); }
bool Symbol::operator==(const Symbol& one) const {
    return ( (this->Named::operator==)(one)
             && (this->Map::operator==)(one) ); }

bool Named::operator<(const Named& other) const
{ return (_name < other._name); }
bool PrimaryMT::operator<(const MathType& other) const {
    if (other.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(other);
        return (getName() < pmt.getName());
    }
    else
        return true;
}
bool ProductMT::operator<(const MathType& other) const {
    if (other.isPrimary())
        return false;
    else {
        try {
            auto& cmt = dynamic_cast<const ProductMT&>(other);
            if (_subTypes.size() != cmt._subTypes.size())
                return (_subTypes.size() < cmt._subTypes.size());
            else {
                for (size_t i = 0; i < _subTypes.size(); ++i)
                    if (*_subTypes[i] < *cmt._subTypes[i])
                        return true;
                return false;
            }
        } catch (std::bad_cast& e)
        { return true; }
    }
}
bool MapMT::operator<(const MathType& other) const {
    if (other.isPrimary())
        return false;
    else {
        try {
            auto& mmt = dynamic_cast<const MapMT&>(other);
            if (*_argsT != *mmt._argsT)
                return (*_argsT < *mmt._argsT);
            else
                return (*_retT < *mmt._retT);
        } catch (std::bad_cast& e)
        { return false; }
    }
}
bool Map::operator<(const Map& other) const
{ return (_type < other._type); }
bool Symbol::operator<(const Symbol& other) const {
    if ((this->Map::operator<)(other))
        return true;
    else if (other.Map::operator<(*this))
        return false;
    else
        return (this->Named::operator<)(other);
}

std::string PrimaryMT::getName() const { return _type; }

std::string ProductMT::getName() const {
    std::stringstream buf;
    auto it = _subTypes.begin(), e = _subTypes.end();
    while (it != e) {
        if ((*it)->isPrimary())
            buf << (*it)->getName();
        else
            buf << '(' << (*it)->getName() << ')';
        ++it;
        if (it != e)
            buf << 'x';
    }
    return buf.str();
}

std::vector<std::string> ProductMT::getNames() const {
    std::vector<std::string> buf;
    for (auto* t : _subTypes)
        buf.push_back(t->getName());
    return buf;
}

std::string MapMT::getName() const {
    std::stringstream buf;
    buf << _argsT->getName() << " -> " << _retT->getName();
    return buf.str();
}

PrimaryMT any_mt("any");
PrimaryMT logical_mt("Logical");

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
        if (!_args[i]->comp(other._args[i]))
            return false;
    return true;
}

bool Variable::comp(const Terms* other) const {
    if (auto v = dynamic_cast<const Variable*>(other))
        return (this->Named::operator==)(*v);
    else
        return false;
}
bool Symbol::comp(const Terms* other) const {
    if (auto s = dynamic_cast<const Symbol*>(other))
        return (*this == *s);
    else
        return false;
}
bool Term::comp(const Terms* other) const {
    if (auto t = dynamic_cast<const Term*>(other))
        return (_f == t->_f && ParenSymbol::operator==(*t));
    else
        return false;
}

const Terms* PrimaryTerm::get(Path path) const
{ return (path.empty() ? this : nullptr); }
const Terms* Term::get(Path path) const {
    if (path.empty())
        return this;
    else {
        auto p = path.top(); path.pop();
        return (p > getArity() ? nullptr : arg(p)->get(path));
    }
}

Terms* PrimaryTerm::replace(Path path, const Terms* by) const
{ return (path.empty() ? by->clone() : nullptr); }
const ParenSymbol::TermsVector&
ParenSymbol::replace(Path path, const Terms* by) const {
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

Term::Term(Symbol f, ParenSymbol::TermsVector args)
        : ParenSymbol(args), _f(f) {
    checkArgs(f, args);

    for (const auto& a : args) {
        if (auto var = dynamic_cast<const Variable*>(a))
            free.insert(*var);
        else {
            auto term = static_cast<const Term*>(a);
            for (const auto& v : term->free)
                free.insert(v);
        }
    }
}

Symbol takeFirstMatchTypes(std::set<Symbol> symSet,
                           const ParenSymbol::TermsVector& args) {
    std::vector<const MathType*> argsType;
    for (const auto& a : args)
        argsType.push_back(a->getType());

    for (const auto& s : symSet)
        if (s.matchArgType(argsType))
            return s;
    throw ParenSymbol::argN_argType_error();
}
Term::Term(std::set<Symbol> symSet, TermsVector args)
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
Symbol forall(Term::qword[Term::QType::FORALL],
              {&any_mt, &logical_mt}, &logical_mt);
Symbol exists(Term::qword[Term::QType::EXISTS],
              {&any_mt, &logical_mt}, &logical_mt);

Terms* PrimaryTerm::replace(const Terms* x, const Terms* t) const
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
    else if (comp(x))
        return t->clone();

    Terms* termReplaced = arg(2)->replace(x, t);
    return new ExistsTerm(
        *static_cast<Variable*>(arg(1)->clone()),
        termReplaced);
}
