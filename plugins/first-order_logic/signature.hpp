//
// Created by anton on 15.12.16.
//

#ifndef TEST_BUILD_SIGNATURE_HPP
#define TEST_BUILD_SIGNATURE_HPP

#include <stdexcept>
#include <map>
#include <list>
#include <set>
#include "logic.hpp"

class Signature
{
public:
    std::map<std::string, unsigned> R;
    std::map<std::string, unsigned> F;
    std::set<std::string> C;

    class sym_sets_intersection : public std::invalid_argument
    {
    public:
        sym_sets_intersection(const std::string& symName)
                : std::invalid_argument("Пересечение множеств символов в конструкторе Signature по символу " + symName +".\n") {}
    };
    class sym_exists : public std::invalid_argument
    {
    public:
        sym_exists(const std::string& symName)
                : std::invalid_argument("Символ с именем \"" + symName + "\" уже определён.\n") {}
    };

    Signature() {}
    Signature(std::list<std::pair<std::string, unsigned> > _R,
              std::list<std::pair<std::string, unsigned> > _F,
              std::list<std::string> _C)
            : R(_R.begin(), _R.end()), F(_F.begin(), _F.end()), C(_C.begin(), _C.end())
    {
        for (auto f : F)
            if (R.find(f.first) != R.end())
                throw sym_sets_intersection(f.first);
        for (auto c : C)
            if (R.find(c) != R.end())
                throw sym_sets_intersection(c);
        for (auto c : C)
            if (F.find(c) != F.end())
                throw sym_sets_intersection(c);
    }
    ~Signature() {}

    unsigned arity(const std::string& name) const
    {
        auto rit = R.find(name);
        if (rit != R.end())
            return rit->second;
        auto fit = F.find(name);
        if (fit != F.end())
            return fit->second;
        return static_cast<unsigned>(-1);
    }

    enum class nameT {predicate, function, constant, none};
    bool checkPredName(const std::string& name) const
    { return (R.find(name) != R.end()); }
    bool checkFuncName(const std::string& name) const
    { return (F.find(name) != F.end()); }
    bool checkConstantName(const std::string& name) const
    { return (C.find(name) != C.end()); }
    nameT checkName(const std::string& name) const
    {
        if (checkPredName(name))
            return nameT::predicate;
        if (checkFuncName(name))
            return nameT::function;
        if (checkConstantName(name))
            return nameT::constant;
        return nameT::none;
    }

    void addPred(const std::string& name, unsigned arity)
    {
        if (checkName(name) != nameT::none)
            throw sym_exists(name);
        R.insert(std::make_pair(name, arity));
    }
    void addFunc(const std::string& name, unsigned arity)
    {
        if (checkName(name) != nameT::none)
            throw sym_exists(name);
        F.insert(std::make_pair(name, arity));
    }
    void addConst(const std::string& name)
    {
        if (checkName(name) != nameT::none)
            throw sym_exists(name);
        C.insert(name);
    }
};

#endif //TEST_BUILD_SIGNATURE_HPP
