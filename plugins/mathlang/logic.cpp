//
// Created by anton on 15.12.16.
//

#include "logic.hpp"

bool Named::operator==(const Named& one) const
{ return (name == one.name); }
bool Label::operator==(const Label& one) const
{ return (this->Named::operator==)(one); }
bool Map::operator==(const Map& one) const
{ return (/*(arity == one.arity) && */(argT == one.argT) && (retT == one.retT)); }
bool Symbol::operator==(const Symbol& one) const
{ return ( (this->Label::operator==)(one) && (this->Map::operator==)(one) ); }
bool MathType::operator==(const MathType& one) const
{
    return (getName() == "any" || one.getName() == "any" ? true
                                                         : (this->Named::operator==)(one));
}

bool Named::operator<(const Named& other) const
{ return (name < other.name); }
bool Label::operator<(const Label& other) const
{ return (this->Named::operator<)(other); }
bool MathType::operator<(const MathType& other) const
{ return (this->Named::operator<)(other); }
bool Map::operator<(const Map& other) const
{
    if (argT < other.argT) return true;
    else if (other.argT < argT) return false;
    else return (retT < other.retT);
}
bool Symbol::operator<(const Symbol& other) const
{
    if ((this->Map::operator<)(other))
        return true;
    else if (other.Map::operator<(*this))
        return false;
    else
        return (this->Label::operator<)(other);
}

MathType logical_mt("Logical");

std::ostream& operator<< (std::ostream& os, const Printable& pr)
{
    pr.print(os);
    return os;
}
void Label::print(std::ostream &out) const
{ out << getName(); }
void ParenSymbol::print(std::ostream &out) const
{
    out << '(';
    if (!args.empty())
    {
        auto lit = args.begin(), le = args.end();
        (*lit)->print(out);
        while (++lit != le)
        {
            out << ", ";
            (*lit)->print(out);
        }
    }
    out << ')';
}
void Term::print(std::ostream &out) const
{
    Symbol::print(out);
    ParenSymbol::print(out);
}
void Term::printQ(std::ostream& out) const
{
    Symbol::print(out);
    auto var = dynamic_cast<const Variable*>(arg(1)); //по пострению arg(1) типа Variable*
    out << var->getName() << "\\typeof " << var->getType().getName();
    arg(2)->print(out);
}


class ParenSymbol::argN_argType_error : public std::invalid_argument
{
public:
    argN_argType_error()
            : std::invalid_argument("Кол-во или тип аргументов не соответствует символу.\n") {}
};

void ParenSymbol::checkArgs(Map f, std::vector<Terms*> _args) const
{
    std::list<MathType> _argsType;
    for (auto a : _args)
        _argsType.push_back(a->getType());
    if (!f.matchArgType(_argsType))
        throw argN_argType_error();
}

ParenSymbol::ParenSymbol(const ParenSymbol& one)
        /*: vars(one.vars)*/
{
    for (const auto& a : one.args)
        args.push_back(a->clone());
}

ParenSymbol::ParenSymbol(std::vector<Terms*> _args)
{
    for (auto a : _args)
        args.push_back(a->clone());
}

ParenSymbol::~ParenSymbol()
{
    for (auto a : args)
        delete a;
}

bool ParenSymbol::operator==(const ParenSymbol& other) const
{
    for (size_t i = 0; i < args.size(); ++i)
        if (!args[i]->doCompare(other.args[i]))
            return false;
    return true;
}

bool Variable::doCompare(const Terms* other) const
{
    if (const Variable* v = dynamic_cast<const Variable*>(other))
        return (this->Label::operator==)(*v);
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

Term::Term(Symbol f, std::vector<Terms*> _args)
        : Terms(f.getType()), Symbol(f), ParenSymbol(_args)
{
    checkArgs(f, _args);

    for (auto& a : _args)
    {
        if (Variable* var = dynamic_cast<Variable*>(a))
            free.insert(*var);
        else if (Term* term = dynamic_cast<Term*>(a))
            for (auto& v : term->free)
                free.insert(v);
    }
}

void Term::boundVar(Variable var)
{
    auto search = free.find(var);
    if (search != free.end())
        free.erase(search);
    else
        throw std::invalid_argument("Попытка ограничения не свободной перменной.\n");
}

std::map<Term::QType, const std::string>
        Term::qword = { {Term::QType::FORALL,"\\forall "},
                        {Term::QType::EXISTS,"\\exists "} };
Symbol forall(Term::qword[Term::QType::FORALL], {MathType("any"), logical_mt}, logical_mt);
Symbol exists(Term::qword[Term::QType::EXISTS], {MathType("any"), logical_mt}, logical_mt);

bool Term::doCompare(const Terms* other) const
{
    if (const Term* t = dynamic_cast<const Term*>(other))
        return ((this->Symbol::operator==)(*t) && (this->ParenSymbol::operator==)(*t));
    else
        return false;
}

Terms* Term::replace(const Terms* x, const Terms* t) const
{
    if (auto var = dynamic_cast<const Variable*>(x))
    {
        if (free.find(*var) == free.end())
            return this->clone();
        else
        {
            std::vector<Terms*> _args;
            for (auto& arg : args)
                _args.push_back(arg->replace(x, t));
            return new Term(*this, _args);
        }
    }
}

Terms* Term::replace(Path where, const Terms* by) const
{
    if (where.size() == 0)
        return by->clone();
    else
    {
        auto n = where.front();
        where.pop_front();
        if (n > args.size())
            return nullptr;
        std::vector<Terms*> _args;
        for (unsigned i = 0; i < args.size(); ++i)
            if (i != n-1)
                _args.push_back(args[i]->clone());
            else
                _args.push_back(args[i]->replace(where, by));
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
