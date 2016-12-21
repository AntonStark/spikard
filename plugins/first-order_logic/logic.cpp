//
// Created by anton on 15.12.16.
//

#include "logic.hpp"

std::ostream& operator<< (std::ostream& os, const Printable& pr)
{
    pr.print(os);
    return os;
}

std::string Named::getName() const
{ return name; }

unsigned Map::getArity() const
{ return arity; }

void Symbol::print(std::ostream &out) const
{ out << getName(); }
void ParenSymbol::print(std::ostream &out) const
{
    out << '(';
    for (unsigned i = 0; i < args.size()-1; ++i)
    { args[i]->print(out); out << ", "; }
    if (!args.empty())
        args.back()->print(out);
    out << ')';
}
void Term::print(std::ostream &out) const
{
    Function::print(out);
    ParenSymbol::print(out);
}
void LOperation::print(std::ostream &out) const
{
    switch (type)
    {
        case LType::NOT : { out << "\\lnot "; break; }
        case LType::AND : { out << "\\land "; break; }
        case LType::OR  : { out << "\\lor ";  break; }
        case LType::THAN: { out << "\\Rightarrow "; break; }
    }
}
void Quantifier::print(std::ostream &out) const
{
    switch (type)
    {
        case QType::FORALL : { out << "\\forall "; break;}
        case QType::EXISTS : { out << "\\exists "; break;}
    }
    arg->print(out);
}
void Formula::print(std::ostream &out) const
{
    if (!arg1)
        return;
    if (!mod)
        arg1->print(out);
    else
    {
        if (!arg2)
        {
            mod->print(out);
            arg1->print(out);
        }
        else
        {
            arg1->print(out);
            mod->print(out);
            arg2->print(out);
        }
    }
}

void Atom::print(std::ostream &out) const
{
    Predicate::print(out);
    ParenSymbol::print(out);
}

bool Named::operator==(const Named &one) const
{ return (name == one.name); }
bool Symbol::operator==(const Symbol &one) const
{ return (this->Named::operator==)(one); }
bool Map::operator==(const Map &one) const
{ return (arity == one.arity); }
bool Predicate::operator==(const Predicate &one) const
{ return ( (this->Symbol::operator==)(one) && (this->Map::operator==)(one) ); }
bool Function::operator==(const Function &one) const
{ return ( (this->Symbol::operator==)(one) && (this->Map::operator==)(one) ); }

Formula::Formula(const std::string& foText)
{
//TODO распознавание формул из строки
}