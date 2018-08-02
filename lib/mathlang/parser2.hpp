//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_PARSER2_HPP
#define SPIKARD_PARSER2_HPP

#include <string>

#include "rationale.hpp"

namespace Parser2
{

struct TeXCommand
{
    const std::string _cmd;

    TeXCommand(const char cmd[]) : _cmd(cmd) {}
    TeXCommand(const std::string& cmd) : _cmd(cmd) {}
    TeXCommand(const TeXCommand& one) = default;

    bool operator< (const TeXCommand& two) const
    { return (_cmd < two._cmd); }
    bool operator== (const TeXCommand& two) const
    { return (_cmd == two._cmd); }
};

extern std::set<TeXCommand> texBrackets;
extern std::map<TeXCommand, TeXCommand>  pairBrackets;

bool isOpenBracket(TeXCommand cmd);

extern std::set<TeXCommand> unprintable;

struct ExpressionLayer
{
    std::vector<TeXCommand> _cmds;
    std::pair<ExpressionLayer*, size_t> _parent;
    unsigned _placeholders;

    ExpressionLayer(ExpressionLayer* parent, size_t indent)
        : _cmds(), _parent({parent, indent}), _placeholders(0) {}

    bool operator< (const ExpressionLayer& two) const
    { return (_placeholders != two._placeholders
              ? _placeholders < two._placeholders
              : _cmds < two._cmds); }
    void insertPlaceholder() {
        _cmds.emplace_back("");
        ++_placeholders;
    }
};

struct CurAnalysisData;

class Lexer
{
//private:
public:
    static void splitToCmds(CurAnalysisData* data);

    static std::pair<size_t, std::string> checkForTexErrors(CurAnalysisData* data);

    static void eliminateUnprintable(CurAnalysisData* data);

    static size_t findFirstBracketFrom(const std::vector<TeXCommand>& inputAsCmds, size_t pos);
    static std::pair<size_t, std::string> findBracketPairs(CurAnalysisData* data);

    static void buildLayerStructure(CurAnalysisData* data, ExpressionLayer* parent, size_t i, size_t bound);
public:
};

/// Контейнер для вспомогательной информации и
/// промежуточных результатов разбора выражения
struct CurAnalysisData
{
    std::string input;
    std::vector<TeXCommand> inputAsCmds;
    std::vector<TeXCommand> inputCmdsPrintable;
    std::map<size_t, size_t> bracketInfo;
    struct comp_by_val {
        bool operator() (ExpressionLayer* const& one,
                         ExpressionLayer* const& two) const
        { return (*one < *two); }
    };
    std::set<ExpressionLayer*, comp_by_val> layers;

    const PrimaryNode* _where;
    Hidden localNames;

    CurAnalysisData(PrimaryNode* where, std::string toParse)
        : _where(where), localNames(where), input(toParse) {
        Lexer::splitToCmds(this);
        Lexer::checkForTexErrors(this);
        Lexer::eliminateUnprintable(this);
        Lexer::findBracketPairs(this); // todo нужно обрабатывать аргументы функций отдельно: разделять по запятым
        Lexer::buildLayerStructure(this, nullptr, 0, inputCmdsPrintable.size());
    }

    void copyCmds(std::vector<TeXCommand>& target, size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i)
            target.emplace_back(inputCmdsPrintable.at(i));
    }
};

CurAnalysisData parse(PrimaryNode* where, std::string toParse);
}

#endif //SPIKARD_PARSER2_HPP
