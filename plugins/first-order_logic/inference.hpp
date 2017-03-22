//
// Created by anton on 15.02.17.
//

#ifndef TEST_BUILD_INFERENCE_HPP
#define TEST_BUILD_INFERENCE_HPP

#include <vector>
#include "signature.hpp"

class Theory : public virtual Printable
{
private:
    std::list<FCard> axioms;
public:
    Theory() : axioms() {}
    Theory(std::initializer_list<FCard> _axioms) : axioms(_axioms) {}
    ~Theory() {}
    void add(FCard axiom)
    { axioms.push_back(axiom); }
    void print(std::ostream& out = std::cout) const override
    {
        for (auto a : axioms)
            out << *a << std::endl;
    }
    const std::list<FCard>& viewAx() const
    { return axioms; }
};

// todo A12 и A13
// todo сохранене вывода (???)
// TODO и как записывать типы переменных? принадлежность категории? (Пусть G - абелева группа...)
class Inference
{
private:
    FormulasFactory& ff;
    typedef std::set<FCard> FCardSet;
    std::map<unsigned, FCard> truth;
//    mutable std::map<unsigned, FCard>::const_iterator index;
    mutable unsigned index;

    std::set<std::pair<FCardSet, FCardSet> > conclusions;

    typedef FCard (Inference::*UnaryTranform)(FCard);
    std::map<unsigned, UnaryTranform> unary;

    typedef FCard (Inference::*BinaryTranform)(FCard, FCard);
    std::map<unsigned, BinaryTranform> binary;

    void logging(std::initializer_list<FCard> in, std::initializer_list<FCard> out);
    void pushTruth(FCard some)
    { truth.insert({truth.size()+1, some}); }
public:
    Inference(FormulasFactory& _ff);
    Inference(FormulasFactory& _ff, const Theory& th);

    FCard A1(FCard A, FCard B);
    FCard A2(FCard AtBtC);
    FCard A3(FCard AaB);
    FCard A4(FCard AaB);
    FCard A5(FCard A, FCard B);
    FCard A6(FCard A, FCard B);
    FCard A7(FCard B, FCard A);
    FCard A8(FCard AtC, FCard B);
    FCard A9(FCard nA, FCard B);
    FCard A10(FCard AtB);
    FCard A11(FCard A);

    void check(FCard f, bool flags[]) const;

    FCard getF(unsigned label) const;

    typedef std::pair<unsigned, std::stack<Formula::ArgTy> > FPath;
    void performUnary(FPath lA, unsigned trans);
    void performBinary(FPath lA, unsigned lB, unsigned trans);

    enum class PrintTy {ALL, INC};
    void print(PrintTy mode, std::ostream& out = std::cout) const;

//    Formula* substitute(const ComposedF* where, Placeholder* what, Formula* by);
};

#endif //TEST_BUILD_INFERENCE_HPP
