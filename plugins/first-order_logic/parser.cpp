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

/*inline bool formulaBegin(const Lexer::Token& tok)
{ return (tok == Lexer::Token::Qf || tok == Lexer::Token::Qe ||
          tok == Lexer::Token::Ln || tok == Lexer::Token::lb || tok == Lexer::Token::P); }

//TODO 1) возможно, через проверку принадлежности multimap<one, two> будет лаконичнее
//TODO 2) ещё можно сделать set<pair<Token, Token> > и проверять принадлежность
//TODO 3) или map<Token, set<Token> >
bool mayFollow(const Lexer::Token& one, const Lexer::Token& two)
{
    switch (one)
    {
        case Lexer::Token::P :
        case Lexer::Token::F :
            return (two == Lexer::Token::lb);
        case Lexer::Token::V :
            return (two == Lexer::Token::c || two == Lexer::Token::rb || formulaBegin(two));
        case Lexer::Token::Ln :
        case Lexer::Token::La :
        case Lexer::Token::Lo :
        case Lexer::Token::Lt :
            return (formulaBegin(two));
        case Lexer::Token::Qf :
        case Lexer::Token::Qe :
            return (formulaBegin(two));
        case Lexer::Token::lb :
            return (two == Lexer::Token::lb || two == Lexer::Token::V ||
                    two == Lexer::Token::F  || formulaBegin(two));
        case Lexer::Token::rb :
            return (two == Lexer::Token::lb || two == Lexer::Token::c ||
                    two == Lexer::Token::La || two == Lexer::Token::Lo || two == Lexer::Token::Lt);
        case Lexer::Token::c :
            return (two == Lexer::Token::V || two == Lexer::Token::F);
        case Lexer::Token::s :
            return true;
    }
}*/

void Parser::markLexems()
{
    PartialResolved pair(0, {});
    stage1.insert(pair);

    size_t newIndent;
    LexList list;
    while (!stage1.empty())
    {
        pair = *stage1.begin();
        if (pair.first == input.length())
            break;
        stage1.erase(stage1.begin());
        bool found = false;
        for (auto w : lex.words)
        {
            if (matchIndented(input, pair.first, w.first))
            {
                found = true;
                newIndent = pair.first + w.first.length();
                list = pair.second;
                list.push_back({w.second, {pair.first, newIndent}});
                stage1.insert({newIndent, list});
                //TODO выводить предупреждение о неоднозначности разбора в данной точке, если в итоге получится больше одной формулы
            }
        }
        if (!found)
        {
            pair.second.push_back({Lexer::Token::V, {pair.first, pair.first+1}});
            stage1.insert({pair.first+1, pair.second});
        }
    }
}

void Parser::mergeVarPieces(LexList& list)
{
    LexList::iterator varBegin, varEnd;
    bool was = false;
    auto e = list.end();
    for (auto it = list.begin(); it != e; ++it)
    {
        if (it->first == Lexer::Token::V)
        {
            if (was)
                varEnd = it;
            else
                varBegin = varEnd = it;
            was = true;
        }
        else
        {
            if (was && varBegin != varEnd)
            {
                //склейка
                varEnd->second.first = varBegin->second.first;
                list.erase(varBegin, varEnd);
            }
            was = false;
        }
    }
}

void Parser::delSpaces(LexList& list)
{
    auto e = list.end();
    for (auto it = list.begin(); it != e; ++it)
        if (it->first == Lexer::Token::s)
            it = list.erase(it);
}

bool checkBrackets(LexList::const_iterator it, LexList::const_iterator e, BracketMap& brackets)
{
    std::stack<LexList::const_iterator> buf;
    for (; it != e; ++it)
    {
        if (it->first == Lexer::Token::lb)
            buf.push(it);
        else if (it->first == Lexer::Token::rb)
        {
            if (buf.empty())
                return false;
            brackets[buf.top()] = it;
            brackets[it] = buf.top();
            buf.pop();
        }
    }
    return (buf.size() == 0);
}

unsigned order(Lexer::Token tok)
{
    switch (tok)
    {
        case Lexer::Token::Ln : return 1;
        case Lexer::Token::La : return 2;
        case Lexer::Token::Lo : return 2;
        case Lexer::Token::Qf : return 3;
        case Lexer::Token::Qe : return 3;
        case Lexer::Token::Lt : return 4;
        default: return 0;
    }
}

void printL(const LexList& list)
{
    for (auto l : list)
        std::cerr << Lexer::tokToStr(l.first) << ":<"
        << l.second.first << ',' << l.second.second << ">" << std::endl;
}

LexList::const_iterator findTopLevelMod(
        const LexList& list)
{
    unsigned priority = 0;
    auto e = list.end();
    auto gotIt = e;
    for (auto it = list.begin(); it != e; ++it)
    {
        if (order(it->first) > priority)
        {
            priority = order(it->first);
            gotIt = it;
        }
    }
    return gotIt;
}

inline std::string Parser::lexToStr(const Lexeme& lex)
{ return input.substr(lex.second.first, lex.second.second - lex.second.first); }




void Parser::recognizeParenSymbol(LexList::const_iterator& it, LexList::const_iterator e, std::list<Terms*>& dst)
{
    //стартуем с открывающей скобки, её пропускаем
    ++it;
    for (; it != e; ++it)
    {
        switch (it->first)
        {
            case Lexer::Token::V :
            { dst.push_back(tf.makeVar(lexToStr(*it))); break; }
            case Lexer::Token::F :
            { dst.push_back(recognizeTerm(it, e)); break; }
            case Lexer::Token::c :
            { break; }
            case Lexer::Token::rb :
            { return; }
            default:
                throw std::invalid_argument("Неправильная скобочная конструкция.\n");
        }
    }
    throw std::invalid_argument("Неправильная скобочная конструкция.\n");
}

Term* Parser::recognizeTerm(LexList::const_iterator& it, LexList::const_iterator end)
{
    Function* f = sigma.getF(lexToStr(*it));
    std::list<Terms*> args;
    auto nit = std::next(it);
    if (nit != end && nit->first == Lexer::Token::lb)
        recognizeParenSymbol(++it, end, args);
    else if (f->getArity() != 0)
        throw std::invalid_argument("Ожидалась скобка.\n");
    return tf.makeTerm(f, args);
}

Formula* Parser::recogniseFormula(const LexList& list)
{
    //F ::= (F) | atom |
    // \\forall x F | \\exists x F | \\lnot F |
    // \\land F | \\lor F | \\Rightarrow F
    auto it = list.begin();
    auto e = list.end();
    BracketMap brackets;
    if (!checkBrackets(it, e, brackets))
        throw std::invalid_argument("Неверная расстановка скобок в формуле \"" + input + "\".\n");
    if (it->first == Lexer::Token::lb && brackets[it] == std::prev(e))
        return recogniseFormula({std::next(it), std::prev(e)});
    auto m = findTopLevelMod(list);
    Modifier* mod;
    Formula *F1, *F2;
    if (m != e)
    {
        if (m->first == Lexer::Token::La ||
            m->first == Lexer::Token::Lo ||
            m->first == Lexer::Token::Lt)
        {
            if (m->first == Lexer::Token::La)
                mod = ff.logAND();
            else if (m->first == Lexer::Token::Lo)
                mod = ff.logOR();
            else
                mod = ff.logTHAN();
            F1 = recogniseFormula({list.begin(), m});
            F2 = recogniseFormula({std::next(m), e});
        }
        else
        {
            F2 = nullptr;
            if (m->first == Lexer::Token::Ln)
            {
                mod = ff.logNOT();
            }
            else
            {
                if ((++m)->first != Lexer::Token::V)
                    throw std::invalid_argument("После квантора должно быть имя переменной.\n");
                Variable* _arg = tf.makeVar(lexToStr(*m));
                if (std::prev(m)->first == Lexer::Token::Qf)
                    mod = ff.forall(_arg);
                else
                    mod = ff.exists(_arg);
            }
            F1 = recogniseFormula({std::next(m), e});
        }
        return ff.makeFormula(mod, F1, F2);
    }
    else
    {
        if (it->first != Lexer::Token::P)
            throw std::invalid_argument("Атом должен начинаться с предиката.\n");
        Predicate* p = sigma.getP(lexToStr(*list.begin()));
        std::list<Terms*> args;
        recognizeParenSymbol(++it, e, args);
        return ff.makeFormula(p, args);
    }
}