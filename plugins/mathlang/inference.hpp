//
// Created by anton on 15.02.17.
//

#ifndef TEST_BUILD_INFERENCE_HPP
#define TEST_BUILD_INFERENCE_HPP

#include <vector>
#include "signature.hpp"

///проблема с указателями Terms
///За
//для наследования удобно использовать указатели
//выгоднее по скорости
//
///Против
//нужно копировать
//new/delete не очень
//fixme пока делаем на указателях, ПОТОМ сделать shared_ptr

//todo Как Theory связана с Reasoning?
//fixme   - Надо наконец отделить интерфейс от реализации,
//fixme     и Theory - одна из реализаций Reasoning
class Theory : public virtual Printable
{
private:
    std::list<Terms*> axioms;
public:
    Theory() : axioms() {}
    Theory(const std::list<Terms*>& _axioms) : axioms(_axioms) {}
    ~Theory() {}
    void add(Terms* axiom)
    { axioms.push_back(axiom); }
    void print(std::ostream& out = std::cout) const override
    {
        for (auto a : axioms)
            out << *a << std::endl;
    }
    const std::list<Terms*>& viewAx() const
    { return axioms; }
};

#endif //TEST_BUILD_INFERENCE_HPP
