//
// Created by anton on 15.02.17.
//

#include "inference.hpp"

Formula* Inference::A2(Formula* F)
{
    if (ComposedF* cF = static_cast<ComposedF*>(F))
    {
        if (cF->getConType() == Modifier::MType::THAN)
        {
            Formula* A = cF->getFArg();
            Formula* BC = cF->getSArg();
            if (ComposedF* cBC = static_cast<ComposedF*>(BC))
            {
                if (cBC->getConType() == Modifier::MType::THAN)
                {
                    Formula* B = cBC->getFArg();
                    Formula* C = cBC->getSArg();

                    Formula* AB = ff.makeFormula(Modifier::MType::THAN, A, B);
                    Formula* AC = ff.makeFormula(Modifier::MType::THAN, A, C);
                    return ff.makeFormula(Modifier::MType::THAN, AB, AC);
                }
            }
        }
    }
    return nullptr;
}

Formula* Inference::A3(Formula* F)
{
    if (ComposedF* cF = static_cast<ComposedF*>(F))
        if (cF->getConType() == Modifier::MType::AND)
            return cF->getFArg();
    return nullptr;
}

Formula* Inference::A4(Formula* F)
{
    if (ComposedF* cF = static_cast<ComposedF*>(F))
        if (cF->getConType() == Modifier::MType::AND)
            return cF->getSArg();
    return nullptr;
}

Formula* Inference::A10(Formula* F)
{
    if (ComposedF* cF = static_cast<ComposedF*>(F))
    {
        if (cF->getConType() == Modifier::MType::THAN)
        {
            Formula* A = cF->getFArg();
            Formula* B = cF->getSArg();

            Formula* nB = ff.makeFormula(MType::NOT, B);
            Formula* AnB = ff.makeFormula(MType::THAN, A, nB);
            Formula* nA = ff.makeFormula(MType::NOT, A);
            return ff.makeFormula(MType::THAN, AnB, nA);
        }
    }
    return nullptr;
}
Formula* Inference::substitute(const Atom* where, const Placeholder* what, const Formula* by)
{ return where; }
//todo с одной стороны так и направашивается полиморфная реализация этой функции внутри иерархии Formula
//todo с другой - это вызовет создание новых формул, а за это отвечает FormulaFactory, к которой нет доступа из Formula
//todo надо понять так ли нужна FormulaFactory? хватит ожидать подвоха от самого себя!
Formula* Inference::substitute(const Placeholder* where, const Placeholder* what, const Formula* by)
{
    if (where == what)
        return by;
    else
        return where;
}

Formula* Inference::substitute(const ComposedF* where, const Placeholder* what, const Formula* by)
{ return ff.makeFormula(where->getConType(),
                        substitute(where->getFArg(), what, by),
                        substitute(where->getSArg(), what, by));

    /*Formula* nArg1;
    if (ComposedF* carg1 = static_cast<ComposedF*>(where->getFArg()))
    {
        Formula::PlacehSet holds = carg1->getHolds();
        if (holds.find(what) != holds.end())
            nArg1 = substitute(carg1, what, by);
        else
            nArg1 = carg1;
    }
    else if (Atom* aarg1 = static_cast<Atom*>(where->getFArg()))
    {
        nArg1 = aarg1;
    }
    else
    {
        Placeholder* parg1 = static_cast<Placeholder*>(where->getFArg());
        if (parg1 == what)
            nArg1 = by;
        else
            nArg1 = parg1;
    }*/

}
