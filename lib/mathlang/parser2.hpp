//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_PARSER2_HPP
#define SPIKARD_PARSER2_HPP

#include <string>
#include <utility>
#include "rationale.hpp"

namespace Parser2
{

struct TexCommand
{
    std::string _cmd;

    TexCommand(const char cmd[]) : _cmd(cmd) {}
    TexCommand(std::string cmd) : _cmd(std::move(cmd)) {}
    TexCommand(const TexCommand&) = default;

    bool operator< (const TexCommand& two) const
    { return (_cmd < two._cmd); }
    bool operator== (const TexCommand& two) const
    { return (_cmd == two._cmd); }
    bool operator!= (const TexCommand& two) const
    { return !(_cmd == two._cmd); }
};

extern std::set<TexCommand> texBrackets;
extern std::map<TexCommand, TexCommand> pairBrackets;
extern std::set<TexCommand> blankCommands;
extern std::set<TexCommand> bracketSizeCommands;

typedef std::vector<TexCommand> TexSequence;

struct ExpressionLayer
{
    TexSequence _cmds;
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
public:
    static TexSequence splitToCmds(CurAnalysisData* data);

    static std::pair<size_t, std::string> checkForTexErrors(CurAnalysisData* data);

    static TexSequence eliminateBracketSizeCommands(const TexSequence& texSequence);
    static TexSequence normalizeBlank(const TexSequence& texSequence);

    static size_t findFirstBracketFrom(const TexSequence& inputAsCmds, size_t pos);
    static std::pair<size_t, std::string> findBracketPairs(CurAnalysisData* data);

    static TexSequence readOneSymbolsCommands(const TexSequence& texSequence);
    static void parseNames(CurAnalysisData* data);

    static void buildLayerStructure(CurAnalysisData* data, ExpressionLayer* parent, size_t i, size_t bound);
};

/// Контейнер для вспомогательной информации и
/// промежуточных результатов разбора выражения
struct CurAnalysisData
{
    std::string input;
    TexSequence inputAsCmds;
    std::vector<TexSequence> asNames;
    std::map<size_t, size_t> bracketInfo;
    struct comp_by_val {
        bool operator() (ExpressionLayer* const& one,
                         ExpressionLayer* const& two) const
        { return (*one < *two); }
    };
    std::set<ExpressionLayer*, comp_by_val> layers;

    const PrimaryNode* _where;
    Hidden localNames;
    std::set<std::string> namesDefined;

    CurAnalysisData(PrimaryNode* where, std::string toParse);

    void copyCmds(TexSequence& target, size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i)
            target.emplace_back(inputAsCmds.at(i));
    }
};

CurAnalysisData parse(PrimaryNode* where, std::string toParse);

}

#endif //SPIKARD_PARSER2_HPP
