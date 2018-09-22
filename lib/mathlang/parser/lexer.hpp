//
// Created by anton on 10.06.18.
//

#ifndef SPIKARD_LEXER2_HPP
#define SPIKARD_LEXER2_HPP

#include <string>
#include <utility>
#include <queue>
#include "map"
#include "set"
#include "stack"

#include "../named_node.hpp"

namespace Parser2
{

enum class Token {w, t, b, l, r,
    ls, rs, lc, rc};

struct LexemStorage
{
    typedef size_t Id;
    typedef unsigned char CatCode;
    std::map<std::string, Id> _dictionary;  // команда -> id
    std::vector<std::string> _index;        // id -> команда
    std::vector<CatCode> _catIndex;         // id -> номер категории
    std::vector<std::string> _catNames;     // номер категории -> категория
    LexemStorage() { _catNames.push_back("general"); }

    /**
     *      нужен функционал:
     * по строке получить id                (при кодировании токенами)
     * по id получить категорию             (в преобразованиях лексера)
     * по id получить команду               (при выводе на печать)
     * задать категорию для множества команд(при настройке лексера)
     *      и желательно синтаксисом: storage[категория] = {команды1, команда2...}
     */

    CatCode _catCode(std::string category);
    Id _store(std::string cmd, unsigned char catCode);
    Id store(std::string cmd) { return _store(std::move(cmd), 0); }

    std::string which(Id id) const { return _catNames[_catIndex[id]]; }
    std::string get(Id id) const { return _index[id]; }

    struct StoreSet
    {
        LexemStorage& _base;
        LexemStorage::CatCode _catCode;
        StoreSet(LexemStorage& base, std::string category)
            : _base(base), _catCode(_base._catCode(std::move(category))) {}
        StoreSet& operator= (const std::set<std::string>& cmdSet) const {
            for (auto& cmd : cmdSet)
                _base._store(cmd, _catCode);
        }
    };
    const StoreSet operator[] (const std::string& category) { return {*this, category}; }
};

struct Lexeme
{
    Token  _tok;
    LexemStorage::Id _id;

    Lexeme(Token structureTok)  : _tok(structureTok), _id(size_t(-1)) {}
    Lexeme(LexemStorage::Id id) : _tok(Token::w), _id(id) {}
    bool operator< (const Lexeme& two) const
    { return (_tok != two._tok ? _tok < two._tok : _id < two._id); }
    bool operator== (const Lexeme& other) const
    { return (_tok != Token::w
                ? _tok == other._tok
                : (other._tok == Token::w) && (_id == other._id) ); }
};
typedef std::vector<Lexeme> LexemeSequence;

/// Слой выражения представляется набором границ интервалов
struct ExpressionLayer
{
    const LexemeSequence& _base;
    const std::pair<size_t, size_t> _bounds;
    std::map<size_t, size_t> _excludes;
    std::map<size_t, size_t> bracketPairs;

    std::pair<ExpressionLayer*, size_t> _parent;

    struct Iter
    {
        const ExpressionLayer& _host;
        size_t pos;
        bool end;

        Iter(const ExpressionLayer& host, size_t i)
            : _host(host), pos(i), end(false) {}
        Iter& operator++ () {
            auto search = _host._excludes.find(pos + 1);
            if (search != _host._excludes.end())
                pos = search->second + 1;
            else
                ++pos;
            end = (pos > _host._bounds.second);
            return *this;
        }
        Iter& operator= (const Iter& other) {
            if (_host._base == other._host._base) {
                pos = other.pos;
                end = other.end;
            }
        }
        Token tok() {
            return ( !end
                     ? _host._base.at(pos)._tok
                     : Token::w );
        }
    };

    ExpressionLayer(const LexemeSequence& base, std::pair<size_t, size_t> bounds,
        ExpressionLayer* parent, size_t indent) : _base(base), _parent({parent, indent}), _bounds(bounds) {}
    ExpressionLayer(const LexemeSequence& base) : ExpressionLayer(base, {0, base.size()-1}, nullptr, 0) {}

    bool operator< (const ExpressionLayer& two) const;

    ExpressionLayer* insertSublayer(std::pair<size_t, size_t> bounds);
    LexemeSequence getLexems();
    Iter begin() { return Iter(*this, _bounds.first); }
};

struct ParseStatus
{
    bool success;
    size_t at;
    std::string mess;
    ParseStatus() : success(true), at(size_t(-1)), mess("") {}
    ParseStatus(size_t at, std::string mess)
        : success(false), at(at), mess(std::move(mess)) {}
};

struct CurAnalysisData;
class Lexer
{
    std::map<std::string, Token> structureSymbols;
    std::map<Token, std::string> tokenPrints;
    void configureLatex();
public:
    LexemStorage storage;
    Lexer() { configureLatex(); }
    ParseStatus splitTexUnits(const std::string& input, LexemeSequence& lexems);
    ParseStatus collectBracketInfo(const LexemeSequence& lexems, std::map<size_t, size_t>& bracketInfo);
    static void buildLayerStructure(CurAnalysisData* data, ExpressionLayer* target);
    static ParseStatus checkRegisters(ExpressionLayer* layer);

    inline std::string print(const Lexeme& l) const
    { return (l._tok == Token::w ? storage.get(l._id) : tokenPrints.at(l._tok)); }
};

/// Контейнер для вспомогательной информации и
/// промежуточных результатов разбора выражения
struct CurAnalysisData
{
    Lexer& _lexer;
    std::string input;
    LexemeSequence lexems;
    std::map<size_t, size_t> bracketInfo;

    template <typename T>
    struct comp_by_val {
        bool operator() (T* const& one, T* const& two) const
        { return (*one < *two); }
    };
    std::set<ExpressionLayer*, comp_by_val<ExpressionLayer> > layers;
    ParseStatus res;

    CurAnalysisData(Lexer& lexer, std::string toParse);
};

CurAnalysisData parse(PrimaryNode* where, std::string toParse);

}

#endif //SPIKARD_LEXER2_HPP
