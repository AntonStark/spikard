//
// Created by anton on 14.01.17.
//

#include "parser.hpp"

bool matchIndented(const std::string& source, const size_t indent, const std::string& word)
{
    if (indent + word.length() > source.length())
        return false;
    else
        return (source.substr(indent, word.length()).compare(word) == 0);
}

MathType parseType(Reasoning& closure, const std::string& source, unsigned& len)
{
    unsigned typeNameLen = 1;
    const Reasoning* reas;
    while ((reas = closure.isNameExist(source.substr(0, typeNameLen),
                                       NameTy::MT)) == nullptr)
        if (typeNameLen == source.length())
            throw std::invalid_argument("Не найдено имя типа - неверный формат.\n");
        else
            ++typeNameLen;
    std::string typeName = source.substr(0, typeNameLen);
    len = typeNameLen;
    return reas->getT(typeName);
}
// Из выражения вида "*\typeof <typeName>"
void registerVar(Reasoning& closure, std::string& source, unsigned indent)
{
    unsigned nameLen = 1;
    while (!matchIndented(source, indent+nameLen, "\\typeof "))
        if (indent+nameLen == source.length())
            throw std::invalid_argument("Отсутствует \"\\\\typeof \" - неверный формат.\n");
        else
            ++nameLen;
    std::string name = source.substr(indent, nameLen);
    unsigned typeNameLen;
    MathType type = parseType(closure, source.substr(indent + nameLen + 8), typeNameLen);
    closure.addVar(name, type);
    source.erase(indent + nameLen, 8+typeNameLen);
}

Lexer::Lexer(Reasoning& _closure) : closure(_closure)
{
    std::set<std::string> buf;
    closure.viewSetOfNames(buf, NameTy::SYM);
    for (auto s : buf)
        words[s] = Token::S;
    buf.clear();
    closure.viewSetOfNames(buf, NameTy::VAR);
    for (auto s : buf)
        words[s] = Token::V;
    buf.clear();
    /*R.viewSetOfNames(buf, NameTy::MT);
    for (auto s : buf)
        words[s] = Token::T;*/

    words[QuantedTerm::word[QuantedTerm::QType::FORALL]]= Token::Q;
    words[QuantedTerm::word[QuantedTerm::QType::EXISTS]]= Token::Q;
    words[","] = Token::c;  words[" "] = Token::s;
    words["("] = Token::lb; words[")"] = Token::rb;
}

//todo работает то?
void Lexer::refreshWords(NameTy type)
{
    Token tok;
    switch (type)
    {
        case NameTy::VAR :
        { tok = Token::V; break; }
        case NameTy::SYM :
        { tok = Token::S; break; }
        case NameTy::MT :
        { /*tok = Token::T; break;*/return; }
    }

    std::set<std::string> buf;
    closure.viewSetOfNames(buf, type);
    // Сначала убираем имена, которые есть и не нужны.
    auto e = words.end();
    auto be = buf.end();
    for (auto it = words.begin(); it != e; ++it)
        if (it->second == tok)      //Имя есть
        {
            auto search = buf.find(it->first);
            if (search == be)       //...и больше ну нужно
                words.erase(it);
            else
                buf.erase(search);  //...и нужно, больше не трогаем
        }
    // Затем добавляем новые.
    for (auto s : buf)
        words[s] = tok;
}

void Lexer::registerVars(std::string& source)
{
    for (unsigned i = 0; i < source.length(); ++i)
        if (matchIndented(source, i, "\\forall ") ||
            matchIndented(source, i, "\\exists "))
            registerVar(closure, source, i+8);
}

inline bool termBegin(const Lexer::Token& tok)
{ return (tok == Lexer::Token::Q || tok == Lexer::Token::lb || tok == Lexer::Token::S); }

//fixme
// 1) возможно, через проверку принадлежности multimap<one, two> будет лаконичнее
// 2) ещё можно сделать set<pair<Token, Token> > и проверять принадлежность
// 3) или map<Token, set<Token> >
bool mayFollow(const Lexer::Token& one, const Lexer::Token& two)
{
    typedef Lexer::Token Token;
    if (two == Lexer::Token::s)
        return true;
    switch (one)
    {
        case Token::S :
            return (two == Token::lb);
        case Token::V :
            return (two == Token::c || two == Token::rb || termBegin(two));
        /*case Token::T :
            return false;*/
        case Token::Q :
            return (two == Token::V);
        case Token::lb :
            return (two == Token::rb || two == Token::V || termBegin(two));
        case Token::rb :
            return (two == Token::rb || two == Token::c);
        case Token::c :
            return (two == Token::V || two == Token::S);
        case Token::s :
            return true;
    }
}

void delSpaces(Lexer::LexList& list)
{
    auto e = list.end();
    for (auto it = list.begin(); it != e; ++it)
        if (it->tok == Lexer::Token::s)
            it = list.erase(it);
}

void Lexer::recognize(std::string source)
{
    registerVars(source);
    refreshWords(NameTy::VAR);

    std::set<PartialResolved> partial;
    PartialResolved pair(0, {});
    partial.insert(pair);

    while (!partial.empty())
    {
        pair = *partial.begin();
        partial.erase(partial.begin());
        if (pair.first == source.length())
        {
            delSpaces(pair.second);
            lastResult.insert(pair.second);
            continue;
        }
        for (auto w : words)
        {
            if ( mayFollow(pair.second.back().tok, w.second)
                  && matchIndented(source, pair.first, w.first))
            {
                size_t newIndent = pair.first + w.first.length();
                LexList list = pair.second;
                list.push_back(w);
                partial.insert({newIndent, list});
            }
        }
    }
}

QuantedTerm* parseQuantedTerm(const Reasoning& reas, Lexer::LexList& list)
{
    typedef Lexer::Token Token;
    QuantedTerm::QType type;
    if (list.front().val == QuantedTerm::word[QuantedTerm::QType::EXISTS])
        type = QuantedTerm::QType::EXISTS;
    else
        type = QuantedTerm::QType::FORALL;
    list.pop_front();

    if (list.front().tok != Token::V)
        return nullptr;
    Variable var = reas.getV(list.front().val);
    list.pop_front();

    Terms* term = parseTerms(reas, list);
    if (term == nullptr)
        return nullptr;
    QuantedTerm* qterm = new QuantedTerm(type, var, term);
    delete term;
    return qterm;
}

Term* parseTerm(const Reasoning& reas, Lexer::LexList& list)
{
    typedef Lexer::Token Token;
    Symbol s = reas.getS(list.front().val);
    list.pop_front();

    if (list.front().tok != Token::lb)
        return nullptr;
    list.pop_front();

    std::list<Terms*> terms;
    while (true)
    {
        if (list.front().tok == Token::rb)
        {
            list.pop_front();
            break;
        }
        Terms* nextT = parseTerms(reas, list);
        if (nextT == nullptr)
            return nullptr;
        terms.push_back(nextT);

        if (list.front().tok == Token::c)
            list.pop_front();
        else if (list.front().tok == Token::rb)
        {
            list.pop_front();
            break;
        }
        else
            return nullptr;
    }
    Term* tterm = new Term(s, terms);
    for (auto t : terms)
        delete t;
    return tterm;
}

Terms* parseTerms(const Reasoning& reas, Lexer::LexList& list)
{
    typedef Lexer::Token Token;
    switch (list.front().tok)
    {
        case Token::Q :
            return parseQuantedTerm(reas, list);
        case Token::S :
            return parseTerm(reas, list);
        case Token::V:
        {
            Variable* var = reas.getV(list.front().val).clone();
            list.pop_front();
            return var;
        }
        case Token::lb:
        {
            list.pop_front();
            Terms* bterm = parseTerms(reas, list);
            list.pop_front();
            return bterm;
        }
        default:
            return nullptr;
    }
}

void addStatement(Reasoning& reas, std::string source)
{
    Lexer lex(reas);
    lex.recognize(source);

    Terms* parsed = nullptr;
    for (auto r : lex.lastResult)
    {
        parsed = parseTerms(reas, r);
        if (parsed != nullptr)
            break;
    }
    //TODO выводить предупреждение о неоднозначности разбора, если в итоге получится больше одного варианта...

    reas.addSub(parsed);
}
