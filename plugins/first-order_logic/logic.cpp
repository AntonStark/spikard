//
// Created by anton on 15.12.16.
//

#include "logic.hpp"

bool Named::operator==(const Named& one) const
{ return (name == one.name); }
bool Symbol::operator==(const Symbol& one) const
{ return (this->Named::operator==)(one); }
bool Map::operator==(const Map& one) const
{ return (arity == one.arity); }
bool Predicate::operator==(const Predicate& one) const
{ return ( /*(sigma.get() == one.sigma.get()) && */(this->Symbol::operator==)(one) && (this->Map::operator==)(one) ); }
bool Function::operator==(const Function &one) const
{ return ( (this->Symbol::operator==)(one) && (this->Map::operator==)(one) ); }




std::ostream& operator<< (std::ostream& os, const Printable& pr)
{
    pr.print(os);
    return os;
}
void Symbol::print(std::ostream &out) const
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
    Function::print(out);
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
void ParenSymbol::argCheck(std::shared_ptr<Map> f, TermsList _args)
{
    if (_args.size() != f->getArity())
        throw nArg_arity_error();
}
ParenSymbol::ParenSymbol(std::list<Terms*> _args)
{
    for (auto t : _args)
    {
        std::shared_ptr<Terms> tSh(t->clone());
        if (std::shared_ptr<Variable> vSh = std::dynamic_pointer_cast<Variable>(tSh))
            vars.insert(vSh);
        args.push_back(tSh);
    }
}
ParenSymbol::ParenSymbol(TermsList _args) : args(_args)
{
    for (auto t : _args)
    {
        if (t->isVariable())
            vars.insert(std::dynamic_pointer_cast<Variable>(t) );
    }
}
