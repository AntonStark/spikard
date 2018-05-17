//
// Created by anton on 25.11.17.
//

#ifndef TEST_BUILD_VIEW_HPP
#define TEST_BUILD_VIEW_HPP

#include "rationale.hpp"

class PlainText : public Representation
{
private:
    std::string value;
    unsigned indent;
    std::string printInd() { return std::string(" ", indent); }
public:
    PlainText() : indent(0) {}
    ~PlainText() override = default;
    std::string str() override { return value; }

    void process(const ListStorage* ls) override;
    void process(const NamedNode* nn) override;
    void process(const DefType* dt) override;
    void process(const DefVar* dv) override;
    void process(const DefSym* ds) override;
    void process(const TermsBox* ax) override;
    void process(const Inference* inf) override;
};

class AsJson : public Representation
{
private:
    json value;
public:
    std::string str() override { return value.dump(2); }

    void process(const ListStorage* ls) override;
    void process(const NamedNode* nn) override;
    void process(const DefType* dt) override;
    void process(const DefVar* dv) override;
    void process(const DefSym* ds) override;
    void process(const TermsBox* ax) override;
    void process(const Inference* inf) override;
};

class AsMlObj : public Representation
{
public:
    struct MlObj
    {
        std::string mlType;
        size_t label;
        std::string body;
        std::vector<std::vector<size_t> > rPremises;

        MlObj(std::string _mlType, size_t _label, std::string _body,
              std::vector<Hierarchy::Path> _premises = {})
            : mlType(std::move(_mlType)), label(_label),
              body(std::move(_body))
        {
            auto reverse = [](Hierarchy::Path straight) -> std::vector<size_t> {
                std::vector<size_t> reversed;
                while (!straight.empty()) {
                    reversed.push_back(straight.top());
                    straight.pop();
                }
                return reversed;
            };
            for (const auto& p : _premises)
                rPremises.push_back(reverse(p));
        }

        json toJson() {
            return json({ {"mlType", mlType},
                          {"label", {label}},
                          {"body", body},
                          {"premises", rPremises} });
        }
    };

    std::list<json> buffer;
    virtual std::string str() override { return "Use .buffer() instead."; }

    void process(const ListStorage* ls) override;
    void process(const NamedNode* nn) override;
    void process(const DefType* dt) override;
    void process(const DefVar* dv) override;
    void process(const DefSym* ds) override;
    void process(const TermsBox* ax) override;
    void process(const Inference* inf) override;
};

#endif //TEST_BUILD_VIEW_HPP
