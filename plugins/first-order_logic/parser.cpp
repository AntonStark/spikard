//
// Created by anton on 14.01.17.
//

#include "parser.hpp"

void prepareForName(std::string& name)
{
    while (name.front() == ' ')
        name.erase(name.begin());
    while (name.back() == ' ')
        name.pop_back();
}
size_t findPairBracket(const std::string& source, size_t pos)
{
    if (source[pos] != '(')
        pos = source.find('(', pos);
    size_t j = pos+1, depth = 1;
    if ((pos == source.npos) || (j == source.npos))
        throw std::invalid_argument("Неверно расставлены скобки в: " + source + ".\n");
    while (j < source.length() && depth > 0)
    {
        if (source[j] == '(')
            ++depth;
        else if (source[j] == ')')
            --depth;
        ++j;
    }
    if (depth == 0 && j != source.npos)
        return j;
    else
        throw std::invalid_argument("Неверно расставлены скобки в: " + source + ".\n");
}
void stripBrackets(std::string& text)
{
    while (text.front() == '(' && text.back() == ')')
    {
        text.pop_back();
        text.erase(text.begin());
    }
}
bool splitByTopLevelLO(std::string source, std::string& left, MType& oper, std::string& right)
{
    std::string local = source;
    size_t i = 0, j;
    while ((i = local.find('(')) != local.npos)
    {
        j = findPairBracket(local, i);
        local.replace(i, j-i, j-i, '_');
    }

    i = local.find("\\Rightarrow");
    if (i != local.npos)
    {
        oper = MType::THAN;
        left = source.substr(0, i);
        right = source.substr(i+11);
        return true;
    }

    i = local.find("\\land");
    j = local.find("\\lor");
    if ((i == local.npos) && (j == local.npos))
        return false;
    else
    {
        if (i<j)
        {
            oper = MType::AND;
            left = source.substr(0, i);
            right = source.substr(i+5);
        }
        else
        {
            oper = MType::OR;
            left = source.substr(0, j);
            right = source.substr(j+4);
        }
        return true;
    }
}


bool matchIndented(const std::string& source, const size_t indent, const std::string& word)
{
    //TODO возможно, здесь нужно сравнивать с учётом отступа
    if (word.length() > source.length())
        return false;
    else
        return (source.substr(indent, word.length()).compare(word) == 0);
}

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
//                Lexeme replace(input, {varBegin->piece.first, varEnd->piece.second}, Lexer::Token::V);
                varEnd->second.first = varBegin->second.first;
//                auto i = list.erase(varEnd); varEnd = list.insert(i, replace);
//                *varEnd = replace;
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

bool checkBrackets(const LexList& list, BracketMap& brackets)
{
    int depth = 0;
    for (const auto& l : list)
    {
        if (l.first == Lexer::Token::lb)
            ++depth;
        else if (l.first == Lexer::Token::rb)
            --depth;
std::cerr << Lexer::tokToStr(l.first) << ":" << depth << std::flush;
    }
    return (depth == 0);
}
LexList::const_iterator fwdPairBracket(LexList::const_iterator it,
                                                 LexList::const_iterator e)
{
    unsigned depth = 0;
    do
    {
        if (it->first == Lexer::Token::lb)
            ++depth;
        else if (it->first == Lexer::Token::rb)
            --depth;
    }
    while (depth != 0 && ++it != e);
    return it;
}

LexList::const_iterator getFirstOf(const Lexer::Token& tok,
                                   LexList::const_iterator b,
                                   LexList::const_iterator e)
{
    for (auto it = b; it != e; ++it)
        if (it->first == tok)
            return it;
        else if (it->first == Lexer::Token::lb)
            it = fwdPairBracket(it, e);
    return e;
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
printL(list);
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
            case Lexer::Token::C :
            { dst.push_back(sigma.getC(lexToStr(*it))); break; }
            case Lexer::Token::V :
            { dst.push_back(sigma.getV(lexToStr(*it))); break; }
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
    Function* f = sigma.getF(lexToStr(*it++));
    std::list<Terms*> args;
    if (it->first != Lexer::Token::lb)
        throw std::invalid_argument("Ожидалась скобка.\n");
    recognizeParenSymbol(it, end, args);
    //TODO эта нестыковка обнаруживает проблему семантики модели: добавление настоящего терма
    // не меняет сигнатуры, т.к. они не часть её (с другой стороны такие же тёрки с переменными),
    // а TermFactory - часть Signature - и приплыли. Пока просто сниму const
    return sigma.makeTerm(f, args);
}

Formula* Parser::recogniseFormula(const LexList& list)
{
    //F ::= (F) | atom |
    // \\forall x F | \\exists x F | \\lnot F |
    // \\land F | \\lor F | \\Rightarrow F
    BracketMap brackets;
    if (!checkBrackets(list, brackets))
        throw std::invalid_argument("Неверная расстановка скобок в формуле \"" + input + "\".\n");
    auto it = list.begin();
    auto e = list.end();
    if (it->first == Lexer::Token::lb && fwdPairBracket(it, e) == --e)
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
                Variable* _arg = sigma.getV(lexToStr(*m));
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