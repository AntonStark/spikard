//
// Created by anton on 15.02.17.
//

#ifndef TEST_BUILD_INFERENCE_HPP
#define TEST_BUILD_INFERENCE_HPP

#include "signature.hpp"

class Inference
{
private:
    FormulasFactory& ff;
public:
    Inference(FormulasFactory& _ff) : ff(_ff) {}

    Formula* A2(Formula* F);
    Formula* A3(Formula* F);
    Formula* A4(Formula* F);

    Formula* A10(Formula* F);

    Formula* substitute(const ComposedF* where, const Placeholder* what, const Formula* by);
    Formula* substitute(const Atom* where, const Placeholder* what, const Formula* by);
    Formula* substitute(const Placeholder* where, const Placeholder* what, const Formula* by);
};

#endif //TEST_BUILD_INFERENCE_HPP
