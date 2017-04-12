//
// Created by anton on 15.12.16.
//

#include "logic.hpp"

bool Named::operator==(const Named& one) const
{ return (name == one.name); }
bool Label::operator==(const Label& one) const
{ return (this->Named::operator==)(one); }
bool Map::operator==(const Map& one) const
{ return ((arity == one.arity) && (argT == one.argT) && (retT == one.retT)); }
bool Symbol::operator==(const Symbol& one) const
{ return ( (this->Label::operator==)(one) && (this->Map::operator==)(one) ); }
/*bool Predicate::operator==(const Predicate& one) const
{ return ( *//*(sigma.get() == one.sigma.get()) && *//*(this->Label::operator==)(one) && (this->Map::operator==)(one) ); }
bool Function::operator==(const Function &one) const
{ return ( (this->Label::operator==)(one) && (this->Map::operator==)(one) ); }*/
bool MathType::operator==(const MathType& other) const
{ return (this->type == other.type); }

bool Named::operator<(const Named& other) const
{ return (name < other.name); }
bool Label::operator<(const Label& other) const
{ return (this->Named::operator<)(other); }
bool MathType::operator<(const MathType& other) const
{ return (type < other.type); }
bool Map::operator<(const Map& other) const
{
    if (argT < other.argT) return true;
    else if (other.argT < argT) return false;

    else if (arity < other.arity) return true;
    else if (other.arity < arity) return false;

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

MathType natural_mt("natural_mt");
MathType logical_mt("logical_mt");
MathType set_mt("set");

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


class ParenSymbol::nArg_arity_error : public std::invalid_argument
{
public:
    nArg_arity_error()
            : std::invalid_argument("Кол-во аргументов не соответствует арности символа.\n") {}
};
void ParenSymbol::argCheck(Map* f, std::list<Terms*> _args)
{
    if (_args.size() != f->getArity())
        throw nArg_arity_error();
}
ParenSymbol::ParenSymbol(std::list<Terms*> _args) : args(_args)
{
    for (auto a : _args)
    {
        if (Variable* v = dynamic_cast<Variable*>(a))
        { vars.insert(v); }
        else if (Term* t = dynamic_cast<Term*>(a))
        { vars.insert(t->vars.begin(), t->vars.end()); }
    }
}
