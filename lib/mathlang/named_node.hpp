//
// Created by anton on 21.09.18.
//

#ifndef SPIKARD_MATHLANG_NAMED_NODE_HPP
#define SPIKARD_MATHLANG_NAMED_NODE_HPP

#include "basics/mathtype.hpp"
#include "basics/terms.hpp"
#include "names.hpp"
#include "structure.hpp"
#include "names_strategies.hpp"

class NamedNode : public Node
{
public:
    enum class NNType {COURSE, SECTION, LECTURE, CLOSURE};
    const NNType _type;
    std::string _title;

    ~NamedNode() override = default;
    NamedNode(Node* parent, NameStoringStrategy* nss,
              NNType type, const std::string& title)
        : Node(parent, nss), _type(type), _title(title) {}
    NamedNode(Node* parent, NameStoringStrategy* nss,
              std::string type, const std::string& title)
        : NamedNode(parent, nss, nntFromStr(type), title) {}

    static std::string typeToStr(NNType nnt);
    static NNType nntFromStr(std::string str);

    std::string getName() const { return _title; }
    void setName(std::string name) { _title = std::move(name); }
    std::string print(Representation* r, bool incremental = true) const override
    { r->process(this); return r->str(); }
};

#endif //SPIKARD_MATHLANG_NAMED_NODE_HPP
