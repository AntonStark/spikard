//
// Created by anton on 04.01.17.
//

#include "formulas.hpp"

void Modifier::print(std::ostream& out) const
{
    out << word[type];
    if (isQuantor(type))
        arg->print(out);
}
void ComposedF::print(std::ostream &out) const
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

std::map<MType, const std::string>
        Modifier::word = { {MType::NOT  , "\\lnot "},
                           {MType::AND  , "\\land "},
                           {MType::OR   , "\\lor " },
                           {MType::THAN , "\\Rightarrow "},
                           {MType::FORALL,"\\forall "},
                           {MType::EXISTS,"\\exists "} };

class Modifier::no_arg : public std::invalid_argument
{
public:
    no_arg() :
            std::invalid_argument("Попытка создания квантора без переменной.\n") {}
};
class Modifier::excess_arg : public std::invalid_argument
{
public:
    excess_arg() :
            std::invalid_argument("Попытка создания логической связки с указанием переменной.\n") {}
};

Modifier::Modifier(MType _type, Variable* _arg)
{
    if (isLogical(_type))
    {
        if (_arg != nullptr)
            throw excess_arg();
        else
            type = _type;
    }
    else
    {
        if (_arg == nullptr)
            throw no_arg();
        else
        {
            type = _type;
            arg = _arg;
        }
    }
}

bool Modifier::isLogical(MType _type) const
{
    return (_type == MType::NOT || _type == MType::AND ||
            _type == MType::OR  || _type == MType::THAN);
}
bool Modifier::isQuantor(MType _type) const
{ return (_type == MType::FORALL || _type == MType::EXISTS); }

bool Modifier::isLogical() const
{ return isLogical(type); }
bool Modifier::isQuantor() const
{ return isQuantor(type); }
bool Modifier::isUnary() const
{
    return (type == MType::NOT ||
            type == MType::FORALL ||
            type == MType::EXISTS);
}

const MType Modifier::getType() const
{ return type; }


/*ComposedF::ComposedF(const Modifier& _mod, const Formula& F)
{
    if (!_mod.isUnary())
        throw std::invalid_argument("Использована не унарная связка.\n");

    mod = _mod.clone();
    arg1 = F.clone();
    arg2 = nullptr;
}
ComposedF::ComposedF(const Modifier& _mod, const Formula& F1, const Formula& F2)
{
    if (_mod.isUnary())
        throw std::invalid_argument("Использована не бинарная связка.\n");

    mod = _mod.clone();
    arg1 = F1.clone();
    arg2 = F2.clone();
}*/

ComposedF::ComposedF(Modifier* _mod, FCard F1, FCard F2)
        : arg1(F1), mod(_mod), arg2(F2)
{
    /*holds = F1->getHolds();
    if (F2)
        for (auto h : F2->getHolds())
            holds.insert(h);*/
}
const Modifier::MType ComposedF::getConType() const
{ return mod->getType(); }
FCard ComposedF::getFArg() const
{ return arg1; }
FCard ComposedF::getSArg() const
{ return arg2; }

FCard Atom::getSub(std::stack<ArgTy> path) const
{
    if (path.size() == 0)
        return this;
    else
        throw std::invalid_argument("Попытка взятия подформулы у атома.");
}
FCard ComposedF::getSub(std::stack<ArgTy> path) const
{
    if (path.size() == 0)
        return this;
    else
    {
        auto arg = path.top();
        path.pop();
        if (arg == ArgTy::f)
            return getFArg()->getSub(path);
        else
            return getSArg()->getSub(path);
    }
}

/*
void ComposedF::substitute(Placeholder* what, Formula* by)
{
    if (holds.find(what) != holds.end())
    {
        if (Placeholder* hArg1 = static_cast<Placeholder*>(arg1))
        {
            if ((*hArg1) == (*what))
                arg1 = by;
        }
        else if (ComposedF* cArg1 = static_cast<ComposedF*>(arg1))
        {
            cArg1->substitute(what, by);
            arg1 = cArg1;
        }
        if (Placeholder* hArg2 = static_cast<Placeholder*>(arg2))
        {
            if ((*hArg2) == (*what))
                arg2 = by;
        }
        else if (ComposedF* cArg2 = static_cast<ComposedF*>(arg2))
        {
            cArg2->substitute(what, by);
            arg2 = cArg2;
        }
    }
}*/
