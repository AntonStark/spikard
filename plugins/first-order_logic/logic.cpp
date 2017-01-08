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
/*
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
            if (arg1->isAtom())
                arg1->print(out);
            else
            {
                out<<'(';
                arg1->print(out);
                out<<')';
            }
        }
        else
        {
            if (arg1->isAtom())
                arg1->print(out);
            else
            {
                out<<'(';
                arg1->print(out);
                out<<')';
            }
            mod->print(out);
            if (arg2->isAtom())
                arg2->print(out);
            else
            {
                out<<'(';
                arg2->print(out);
                out<<')';
            }
        }
    }
}
void Atom::print(std::ostream &out) const
{
    Predicate::print(out);
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


Formula::Formula(const Modifier& _mod, const Formulas& F)
{
    if (_mod.isLOperation() && (_mod.getType() != 0) )
        throw std::invalid_argument("Использована не унарная операция.\n");

    mod = std::shared_ptr<Modifier>(_mod.clone());
    arg1 = std::shared_ptr<Formulas>(F.clone());
    arg2 = nullptr;
}
Formula::Formula(const LOperation& _mod, const Formulas& F1, const Formulas& F2)
{
    if (_mod.getType() == 0)
        throw std::invalid_argument("Отрицание - не бинарная операция.\n");

    mod = std::shared_ptr<Modifier>(_mod.clone());
    arg1 = std::shared_ptr<Formulas>(F1.clone());
    arg2 = std::shared_ptr<Formulas>(F2.clone());
}
*/
