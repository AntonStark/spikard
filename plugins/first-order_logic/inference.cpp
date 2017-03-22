//
// Created by anton on 15.02.17.
//

#include "inference.hpp"

Inference::Inference(FormulasFactory& _ff) : ff(_ff)
{
    index = 0;

    unary[2] = &Inference::A2;
    unary[3] = &Inference::A3;
    unary[4] = &Inference::A4;
    unary[10]= &Inference::A10;
    unary[11]= &Inference::A11;

    binary[1] = &Inference::A1;
    binary[5] = &Inference::A5;
    binary[6] = &Inference::A6;
    binary[7] = &Inference::A7;
    binary[8] = &Inference::A8;
    binary[9] = &Inference::A9;

    /*A[12] = A12;
    A[13] = A13;*/
}
Inference::Inference(FormulasFactory& _ff, const Theory& th)
        : Inference(_ff)
{
    for (auto o : th.viewAx())
        pushTruth(o);
}

FCard Inference::A1(FCard A, FCard B)
{
    if (A != nullptr && B != nullptr)
        return ff.makeFormula(MType::THAN, A, B);
    else
        return nullptr;
}

FCard Inference::A2(FCard AtBtC)
{
    if (const ComposedF* cAtBtC = static_cast<const ComposedF*>(AtBtC))
    {
        if (cAtBtC->getConType() == MType::THAN)
        {
            FCard A = cAtBtC->getFArg();
            FCard BtC = cAtBtC->getSArg();
            if (const ComposedF* cBtC = static_cast<const ComposedF*>(BtC))
            {
                if (cBtC->getConType() == MType::THAN)
                {
                    FCard B = cBtC->getFArg();
                    FCard C = cBtC->getSArg();

                    FCard AtB = ff.makeFormula(MType::THAN, A, B);
                    FCard AtC = ff.makeFormula(MType::THAN, A, C);
                    return ff.makeFormula(MType::THAN, AtB, AtC);
                }
            }
        }
    }
    return nullptr;
}

FCard Inference::A3(FCard AaB)
{
    if (const ComposedF* cAaB = static_cast<const ComposedF*>(AaB))
        if (cAaB->getConType() == MType::AND)
            return cAaB->getFArg();
    return nullptr;
}

FCard Inference::A4(FCard AaB)
{
    if (const ComposedF* cAaB = static_cast<const ComposedF*>(AaB))
        if (cAaB->getConType() == MType::AND)
            return cAaB->getSArg();
    return nullptr;
}

FCard Inference::A5(FCard A, FCard B)
{
    if (A != nullptr && B != nullptr)
    {
        FCard AaB = ff.makeFormula(MType::AND, A, B);
        return ff.makeFormula(MType::THAN, B, AaB);
    }
    else
        return nullptr;
}

FCard Inference::A6(FCard A, FCard B)
{
    if (A != nullptr && B != nullptr)
        return ff.makeFormula(MType::OR, A, B);
    else
        return nullptr;
}

FCard Inference::A7(FCard B, FCard A)
{
    if (A != nullptr && B != nullptr)
        return ff.makeFormula(MType::OR, A, B);
    else
        return nullptr;
}

//Formula* Inference::substitute(const ComposedF* where, Placeholder* what, Formula* by)
//{
//    ComposedF* cF = static_cast<ComposedF*>(ff.makeFormula(where));
//    cF->substitute(what, by);
//    return cF;
//}

FCard Inference::A8(FCard AtC, FCard B)
{
    if (const ComposedF* cAtC = static_cast<const ComposedF*>(AtC))
    {
        if (cAtC->getConType() == MType::THAN)
        {
            FCard A = cAtC->getFArg();
            FCard C = cAtC->getSArg();

            FCard BtC = ff.makeFormula(MType::THAN, B, C);
            FCard AoB = ff.makeFormula(MType::OR, A, B);
            FCard AoBtC = ff.makeFormula(MType::THAN, AoB, C);
            return ff.makeFormula(MType::THAN, BtC, AoBtC);
        }
    }
    return nullptr;
}

FCard Inference::A9(FCard nA, FCard B)
{
    if (const ComposedF* cnA = static_cast<const ComposedF*>(nA))
    {
        if (cnA->getConType() == MType::NOT)
        {
            FCard A = cnA->getFArg();
            return ff.makeFormula(MType::THAN, A, B);
        }
    }
    return nullptr;
}

FCard Inference::A10(FCard AtB)
{
    if (const ComposedF* cAtB = static_cast<const ComposedF*>(AtB))
    {
        if (cAtB->getConType() == MType::THAN)
        {
            FCard A = cAtB->getFArg();
            FCard B = cAtB->getSArg();

            FCard nB = ff.makeFormula(MType::NOT, B);
            FCard AtnB = ff.makeFormula(MType::THAN, A, nB);
            FCard nA = ff.makeFormula(MType::NOT, A);
            return ff.makeFormula(MType::THAN, AtnB, nA);
        }
    }
    return nullptr;
}

FCard Inference::A11(FCard A)
{
    if (A != nullptr)
    {
        FCard nA = ff.makeFormula(MType::NOT, A);
        return ff.makeFormula(MType::OR, A, nA);
    }
    else
        return nullptr;
}

void Inference::logging(std::initializer_list<FCard> in, std::initializer_list<FCard> out)
{
    conclusions.insert(std::make_pair(in, out));
    for (auto o : out)
        pushTruth(o);
}

void Inference::check(FCard f, bool flags[10]) const
{
    for (int i = 0; i < 10; ++i)
        flags[i] = false;
    flags[0] = flags[4] = flags[5] = flags[6] = true;
    if (auto cf = static_cast<const ComposedF*>(f))
    {
        auto mod = cf->getConType();
        if (mod == MType::NOT)
            flags[8] = true;
        else if (mod == MType::AND)
            flags[2] = flags[3] = true;
        else if (mod == MType::THAN)
        {
            flags[7] = flags[9] = true;
            if (auto cg = static_cast<const ComposedF*>(cf->getSArg()))
            {
                if (cg->getConType() == MType::THAN)
                    flags[1] = true;
            }
        }
    }
}

FCard Inference::getF(unsigned label) const
{
    auto search = truth.find(label);
    if (search != truth.end())
        return search->second;
    else
        return nullptr;
}

void Inference::performUnary(FPath lA, unsigned trans)
{
    auto tr = unary.find(trans);
    if (tr == unary.end())
        return;
    FCard A = getF(lA.first);
    FCard pA = getF(lA.first)->getSub(lA.second);
    FCard apply = (this->*(tr->second))(pA);
    if (apply != nullptr)
    {
        FCard res = ff.makeFormula(A, lA.second, apply);
        logging({A}, {res});
    }
}
void Inference::performBinary(FPath lA, unsigned lB, unsigned trans)
{
    auto tr = binary.find(trans);
    if (tr == binary.end())
        return;
    FCard A = getF(lA.first);
    FCard pA = getF(lA.first)->getSub(lA.second);
    FCard B = getF(lB);
    FCard apply = (this->*(tr->second))(pA, B);
    if (apply != nullptr)
    {
        FCard res = ff.makeFormula(A, lA.second, apply);
        logging({A, B}, {res});
    }
}

inline void printItem(const std::pair<unsigned, FCard>& item, std::ostream& out)
{
    out << '[' << item.first << "]: "
    << *item.second << std::endl;
}
void Inference::print(PrintTy mode, std::ostream& out) const
{
    if (mode == PrintTy::ALL)
        for (auto& i : truth)
            printItem(i, out);
    else
    {
        auto e = truth.end();
        auto it = truth.upper_bound(index);
        while (it != e)
            printItem(*it++, out);
        index = truth.size();
    }
}
