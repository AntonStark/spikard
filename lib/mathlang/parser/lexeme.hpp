//
// Created by anton on 30.10.18.
//

#ifndef SPIKARD_LEXEME_HPP
#define SPIKARD_LEXEME_HPP

#include <map>
#include <vector>
#include <set>

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
    Id _store(const std::string& cmd, unsigned char catCode);
    Id store(const std::string& cmd) { return _store(cmd, 0); }

    std::string which(Id id) const { return _catNames[_catIndex[id]]; }
    std::string get(Id id) const { return _index[id]; }

    struct StoreSet
    {
        LexemStorage& _base;
        LexemStorage::CatCode _catCode;
        StoreSet(LexemStorage& base, std::string category)
            : _base(base), _catCode(_base._catCode(std::move(category))) {}
        StoreSet& operator= (const std::set<std::string>& cmdSet) {
            for (auto& cmd : cmdSet)
                _base._store(cmd, _catCode);
            return *this;
        }
    };
    StoreSet operator[] (const std::string& category)
    { return {*this, category}; }
};

struct Lexeme
{
    Token  _tok;
    LexemStorage::Id _id;
    unsigned _originOffset;

    Lexeme(Token structureTok, unsigned originOffset)
    : _tok(structureTok), _id(size_t(-1)), _originOffset(originOffset) {}
    Lexeme(LexemStorage::Id id, unsigned originOffset)
    : _tok(Token::w), _id(id), _originOffset(originOffset) {}
    bool operator< (const Lexeme& two) const
    { return (_tok != two._tok ? _tok < two._tok : _id < two._id); }
    bool operator== (const Lexeme& other) const
    { return (_tok != Token::w
              ? _tok == other._tok
              : (other._tok == Token::w) && (_id == other._id) ); }
    bool operator!= (const Lexeme& other) const
    { return (!operator==(other)); }
};
typedef std::vector<Lexeme> LexemeSequence;

}

#endif //SPIKARD_LEXEME_HPP
