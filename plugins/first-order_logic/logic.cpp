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
{ return (this->Named::operator==)(one); }

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

    /*else if (arity < other.arity) return true;
    else if (other.arity < arity) return false;*/

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
void QuantedTerm::print(std::ostream &out) const
{ out << word[type] << var << *term; }


class ParenSymbol::argN_argType_error : public std::invalid_argument
{
public:
    argN_argType_error()
            : std::invalid_argument("Кол-во или тип аргументов не соответствует символу.\n") {}
};
void ParenSymbol::argCheck(Map f, std::list<std::reference_wrapper<Terms> > _args)
{
    std::list<MathType> _argsType;
    for (auto a : _args)
        _argsType.push_back(a.get().getType());
    if (!f.matchArgType(_argsType))
        throw argN_argType_error();
}
void ParenSymbol::argCheck(Map f, std::list<Terms*> _args)
{
    std::list<MathType> _argsType;
    for (auto a : _args)
        _argsType.push_back(a->getType());
    if (!f.matchArgType(_argsType))
        throw argN_argType_error();
}
ParenSymbol::ParenSymbol(std::list<std::reference_wrapper<Terms> > _args)
{
    for (auto& a : _args)
    {
        Terms& ta = a.get();
        args.push_back(ta.clone());
        if (ta.isVariable())
        {
            Variable v = static_cast<Variable&>(ta);
            vars.insert(v);
        }
        else
        {
            Term t = static_cast<Term&>(ta);
            vars.insert(t.vars.begin(), t.vars.end());
        }
    }
}

ParenSymbol::ParenSymbol(const ParenSymbol& one)
        : vars(one.vars)
{
    for (const auto& a : one.args)
        args.push_back(a->clone());
}

ParenSymbol::ParenSymbol(std::list<Terms*> _args)
{
    for (auto a : _args)
    {
        args.push_back(a->clone());
        if (Variable* v = dynamic_cast<Variable*>(a))
            vars.insert(*v);
        else if (Term* t = dynamic_cast<Term*>(a))
            vars.insert(t->vars.begin(), t->vars.end());
    }
}

ParenSymbol::~ParenSymbol()
{
    for (auto a : args)
        delete a;
}

std::map<QuantedTerm::QType, const std::string>
        QuantedTerm::word = { {QuantedTerm::QType::FORALL,"\\forall "},
                              {QuantedTerm::QType::EXISTS,"\\exists "} };
