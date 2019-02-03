//
// Created by anton on 20.01.19.
//

#include "complex.hpp"

bool ComplexTerm::comp(const AbstractTerm* other) const {
    if (auto otherComplex = dynamic_cast<const ComplexTerm*>(other))
        return (_symbol == otherComplex->_symbol && _args == otherComplex->_args);
    else
        return false;
}
const AbstractTerm* ComplexTerm::get(AbstractTerm::Path path) const {
    if (path.empty())
        return this;
    else {
        auto p = path.top(); path.pop();
        return (p > _symbol->getArity() ? nullptr : arg(p)->get(path));
    }
}
AbstractTerm::Vector replace(const AbstractTerm::Vector& args, AbstractTerm::Path path, const AbstractTerm* by) {
    AbstractTerm::Vector updated;
    auto p = path.top();
    path.pop();
    for (size_t i = 0; i < args.size(); ++i)
        updated.push_back(i == p - 1 ? args[i]->replace(path, by) : args[i]->clone());
    return updated;
}
AbstractTerm* ComplexTerm::replace(AbstractTerm::Path path, const AbstractTerm* by) const {
    if (path.empty())
        return by->clone();
    if (path.top() > _symbol->getArity())
        return nullptr;
    return new ComplexTerm(_symbol, ::replace(_args, path, by));
}
