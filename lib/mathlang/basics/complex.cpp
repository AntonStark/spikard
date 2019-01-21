//
// Created by anton on 20.01.19.
//

#include "complex.hpp"

const AbstractConnective* Complex::getSym() const
{ return _symbol; }

const MathType* Complex::getType() const
{ return _symbol->resultType(); }

std::string Complex::print() const
{ _symbol->print(_args); }

bool Complex::comp(const Terms* other) const {
    if (auto otherComplex = dynamic_cast<const Complex*>(other))
        return (_symbol == otherComplex->_symbol && _args == otherComplex->_args);
    else
        return false;
}

Terms* Complex::clone() const
{ return new Complex(*this); }

const Terms* Complex::arg(size_t oneTwoThree) const
{ return _args.at(oneTwoThree-1); }

const Terms* Complex::get(Terms::Path path) const {
    if (path.empty())
        return this;
    else {
        auto p = path.top(); path.pop();
        return (p > _symbol->getArity() ? nullptr : arg(p)->get(path));
    }
}

TermsVector replace(TermsVector args, Terms::Path path, const Terms* by) {
    TermsVector updated;
    auto p = path.top();
    path.pop();
    for (size_t i = 0; i < args.size(); ++i)
        updated.push_back(i == p - 1 ? args[i]->replace(path, by) : args[i]->clone());
    return updated;
}

Terms* Complex::replace(Terms::Path path, const Terms* by) const {
    if (path.empty())
        return by->clone();
    if (path.top() > _symbol->getArity())
        return nullptr;
    return new Complex(_symbol, ::replace(_args, path, by));
}

Terms* Complex::replace(const Terms* x, const Terms* t) const
{ return nullptr; } // fixme заглушка, убрать вовсе
