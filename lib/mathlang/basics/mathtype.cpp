//
// Created by anton on 17.05.18.
//

#include "mathtype.hpp"

MathType* typeOfTypes = new PrimaryMT(new TexName("Type"));
PrimaryMT any_mt(new TexName("any"));
PrimaryMT logical_mt(new TexName("Logical"));

bool PrimaryMT::operator==(const MathType& one) const {
    if (getName() == "any")
        return true;
    if (one.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(one);
        return (pmt.getName() == "any" || getName() == pmt.getName());
    }
    else
        return false;
}

bool PrimaryMT::operator<(const MathType& other) const {
    if (other.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(other);
        return (getName() < pmt.getName());
    }
    else
        return true;
}

const MathType* PrimaryMT::getType() const
{ return typeOfTypes; }

std::string PrimaryMT::print() const
{ return getName(); }

bool ProductMT::operator==(const MathType& one) const {
    if (one.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(one);
        return (pmt.getName() == "any");
    }
    else {
        // todo избавиться от try-catch
        try
        {
            auto& pmt = dynamic_cast<const ProductMT&>(one);
            if (_subTypes.size() != pmt._subTypes.size())
                return false;
            for (size_t i = 0; i < _subTypes.size(); ++i)
                if (*_subTypes[i] != *pmt._subTypes[i])
                    return false;
            return true;
        } catch (std::bad_cast& e)
        { return false; }
    }
}

bool ProductMT::operator<(const MathType& other) const {
    if (other.isPrimary())
        return false;
    else {
        try {
            auto& cmt = dynamic_cast<const ProductMT&>(other);
            if (_subTypes.size() != cmt._subTypes.size())
                return (_subTypes.size() < cmt._subTypes.size());
            else {
                for (size_t i = 0; i < _subTypes.size(); ++i)
                    if (*_subTypes[i] < *cmt._subTypes[i])
                        return true;
                return false;
            }
        } catch (std::bad_cast& e)
        { return true; }
    }
}

MathType* ProductMT::clone() const {
    MTVector newSubTypes;
    for (const auto& t : _subTypes)
        newSubTypes.push_back(t->clone());
    return new ProductMT(newSubTypes);
}

std::string ProductMT::getName() const {
    std::stringstream buf;
    auto it = _subTypes.begin(), e = _subTypes.end();
    while (it != e) {
        if ((*it)->isPrimary())
            buf << (*it)->getName();
        else
            buf << '(' << (*it)->getName() << ')';
        ++it;
        if (it != e)
            buf << 'x';
    }
    return buf.str();
}

std::vector<std::string> ProductMT::getNames() const {
    std::vector<std::string> buf;
    for (auto* t : _subTypes)
        buf.push_back(t->getName());
    return buf;
}

bool ProductMT::matchArgType(const MTVector& otherMTV) const {
    if (otherMTV.size() != _subTypes.size())
        return false;
    for (size_t i = 0; i < _subTypes.size(); ++i)
        if (*otherMTV[i] != *_subTypes[i])
            return false;
    return true;
}

bool MapMT::operator==(const MathType& one) const {
    if (one.isPrimary()) {
        auto& pmt = dynamic_cast<const PrimaryMT&>(one);
        return (pmt.getName() == "any");
    }
    else {
        try
        {
            auto& mmt = dynamic_cast<const MapMT&>(one);
            return (*_argsT == *mmt._argsT && *_retT == *mmt._retT);
        } catch (std::bad_cast& e)
        { return false; }
    }
}

bool MapMT::operator<(const MathType& other) const {
    if (other.isPrimary())
        return false;
    else {
        try {
            auto& mmt = dynamic_cast<const MapMT&>(other);
            if (*_argsT != *mmt._argsT)
                return (*_argsT < *mmt._argsT);
            else
                return (*_retT < *mmt._retT);
        } catch (std::bad_cast& e)
        { return false; }
    }
}

std::string MapMT::getName() const {
    std::stringstream buf;
    buf << _argsT->getName() << " -> " << _retT->getName();
    return buf.str();
}

/*const MathType* MathType::getType() const
{ return typeOfTypes; }

std::string MathType::print() const
{ return getName(); }*/


//bool MathType::comp(const Terms* other) const { return false; }
//const Terms* MathType::get(Terms::Path path) const { return nullptr; }
//Terms* MathType::replace(Terms::Path path, const Terms* by) const { return nullptr; }
//Terms* MathType::replace(const Terms* x, const Terms* t) const { return nullptr; }
