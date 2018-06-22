//
// Created by anton on 10.06.18.
//

#include "parser2.hpp"

namespace Parser2 {

std::set<TeXCommand> texBrackets =
    {"{", "}", "(", ")", "[", "]"};
std::map<TeXCommand, TeXCommand>  pairBrackets =
    { {"{", "}"}, {"(", ")"}, {"[", "]"} };


void Lexer::collectWords() {
    for (auto& s : _where->index().getNames(NameTy::SYM))
        words[s] = Token::N;
    for (const auto& s : _where->index().getNames(NameTy::VAR))
        words[s] = Token::N;
    for (const auto& s : _where->index().getNames(NameTy::CONST))
        words[s] = Token::N;
    words[ Term::qword[Term::QType::FORALL] ] = Token::N;
    words[ Term::qword[Term::QType::EXISTS] ] = Token::N;

    words[","] = Token::c;  words[" "] = Token::s;
    words["("] = Token::lb; words[")"] = Token::rb;
    words["{"] = Token::lc; words["}"] = Token::rc;
    words["^"] = Token::t;  words["_"] = Token::b;
}

void Lexer::splitToCmds() {
    size_t j, i = 0;
    while (i < input.length()) {
        j = i + 1;
        if (input.at(i) == '\\') {  // команда или экранированный символ
            while (j < input.length() && std::isalpha(static_cast<unsigned char>(input.at(j))))
                ++j;
            if (j == i + 1)         // следом за \ идёт не буква, а экранированный символ
                ++j;
        }

        inputAsCmds.emplace_back(input.substr(i, j-i));
        i = j;
    }
}

std::pair<size_t, std::string> Lexer::checkForTexErrors() {
    //todo двойное __, ^^, хвостовой \, проблемы с {}
}

size_t Lexer::findFirstBracketFrom(size_t pos) {
    while (pos < inputAsCmds.size())
        if (texBrackets.find(inputAsCmds[pos]) != texBrackets.end())
            return pos;
        else
            ++pos;
    return size_t(-1);
}

bool Lexer::isOpenBracket(TeXCommand cmd)
{ return (pairBrackets.find(cmd) != pairBrackets.end()); }

std::pair<size_t, std::string> Lexer::findBracketPairs() {
    std::stack<std::pair<TeXCommand, size_t> > opened;
    size_t i = 0;
    while ((i = findFirstBracketFrom(i)) != size_t(-1)) {
        TeXCommand iCmd = inputAsCmds[i];
        if (isOpenBracket(iCmd))
            opened.emplace(iCmd, i);
        else if (iCmd == pairBrackets.at(opened.top().first)) {
            bracketInfo.emplace(opened.top().second, i);
            opened.pop();
        }
        else
            return {i, "Ошибка: закрывающая скобка " + iCmd._cmd +
                " (" + std::to_string(i) + "-ая TeX-команда) не имеет пары."};
        ++i;
    }
    if (not opened.empty())
        return {opened.top().second, "Ошибка: не найдена закрывающая скобка для " + opened.top().first._cmd +
            " (" + std::to_string(opened.top().second) + "-ая TeX-команда)."};
    return {0, ""};
}

}
