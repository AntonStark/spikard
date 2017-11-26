//
// Created by anton on 25.11.17.
//

#ifndef TEST_BUILD_VIEW_HPP
#define TEST_BUILD_VIEW_HPP

#include "rationale.hpp"

class PlainText : public Representation
{
private:
    unsigned indent;
    std::string printInd() { return std::string(" ", indent); }
public:
    std::string process(const ListStorage* ls) override;
    std::string process(const NamedNode* nn) override;
    std::string process(const DefType* dt) override;
    std::string process(const DefVar* dv) override;
    std::string process(const DefSym* ds) override;
    std::string process(const Axiom* ax) override;
    std::string process(const AbstrInf* ai) override;
};

class AsJson : public Representation
{
public:
    std::string process(const ListStorage* ls) override;
    std::string process(const NamedNode* nn) override;
    std::string process(const DefType* dt) override;
    std::string process(const DefVar* dv) override;
    std::string process(const DefSym* ds) override;
    std::string process(const Axiom* ax) override;
    std::string process(const AbstrInf* ai) override;
};

#endif //TEST_BUILD_VIEW_HPP
