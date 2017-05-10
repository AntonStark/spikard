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

inline bool termBegin(const Lexer::Token& tok)
{ return (tok == Lexer::Token::Q || tok == Lexer::Token::lb || tok == Lexer::Token::S); }

// 1) возможно, через проверку принадлежности multimap<one, two> будет лаконичнее
// 2) ещё можно сделать set<pair<Token, Token> > и проверять принадлежность
// 3) или map<Token, set<Token> >
bool mayFollow(const Lexer::Token& one, const Lexer::Token& two)
{
    if (two == Lexer::Token::s)
        return true;
    switch (one)
    {
        case Lexer::Token::S :
            return (two == Lexer::Token::lb);
        case Lexer::Token::V :
            return (two == Lexer::Token::c || two == Lexer::Token::rb || termBegin(two));
        /*case Lexer::Token::T : //todo что там с выражениями вида "Пусть G - св. ред. группа"
            return false;*/
        case Lexer::Token::Q :
            return (two == Lexer::Token::V);
        case Lexer::Token::lb :
            return (two == Lexer::Token::rb || two == Lexer::Token::V || termBegin(two));
        case Lexer::Token::rb :
            return (two == Lexer::Token::rb || two == Lexer::Token::c);
        case Lexer::Token::c :
            return (two == Lexer::Token::V || two == Lexer::Token::S);
        case Lexer::Token::s :
            return true;
    }
}
//fixme 1) Квантификация терма превращает его в логический терм? Ведь не может быть add(1, \forall x !=(x, 0))
//      1) Хотя квантифицируются и так только логические термы. Можно ввести class QuantedTerm - DONE
//         Приписывание квантора не меняет типа => разрешено использование внутри скобок. Важен только порядок кванторов
//fixme 2) Что там с выражениями вида "Пусть G - св. ред. группа". Эта конструкция просто вводит в область имён терм заданного типа
//      2) Сама же она термом не является и требует отдельного расмотрения
//fixme 3) Кванторы тоже являются конструкциями ввода имен. После встречи с квантором надо считывать имя переменной по символам
//         пока не встретится что-то другое. В остальных ситуациях имеем дело с известными именами.
//fixme 3`)После квантора конструкция вида "\delta\typeof Real" -- это назывное
//         утверждение на уровень ниже, порождающее переменную (терм) заданного типа.
void Lexer::delSpaces(LexList& list)
{
    auto e = list.end();
    for (auto it = list.begin(); it != e; ++it)
        if (it->second == Lexer::Token::s)
            it = list.erase(it);
}

void Lexer::recognize(const std::string& source)
{
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
            if ( mayFollow(pair.second.back().second, w.second)
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
/*

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
            */
/*brackets[buf.top()] = it;
            brackets[it] = buf.top();*//*

            brackets.insert(std::make_pair(buf.top(), it));
            brackets.insert(std::make_pair(it, buf.top()));
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
void printLlciter(const Llciter& it)
{ std::cout << "iter_to:" << Lexer::tokToStr(it->first) << "(on_sym=" << it->second.first << ")"; }
void printBM(const BracketMap& brackes)
{
    auto e = brackes.end();
    for (auto it = brackes.begin(); it != e; ++it)
    {
        std::cout << "Pair[";
        printLlciter(it->first);
        std::cout << ", ";
        printLlciter(it->second);
        std::cout << "]" << std::endl;
    }
}

Llciter findTopLevelMod(const LexList& list, const BracketMap& brackets)
{
    unsigned priority = 0;
    Llciter it = list.begin();
    Llciter e = list.end();
    Llciter gotIt = e;
    while (it != e)
    {
        if (it->first == Lexer::Token::lb)
            it = brackets.at(it);
        else if (order(it->first) > priority)
        {
            priority = order(it->first);
            gotIt = it;
        }
        ++it;
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

const Formula* Parser::recogniseFormula(const LexList& list)
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
    auto m = findTopLevelMod(list, brackets);
    Modifier* mod;
    const Formula *F1, *F2;
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
}*/

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

void registerVars(Reasoning& closure, std::string& source)
{
    for (unsigned i = 0; i < source.length(); ++i)
        if (matchIndented(source, i, "\\forall ") ||
            matchIndented(source, i, "\\exists "))
            registerVar(closure, source, i+8);
}


Terms* parseTerm(const Reasoning& reas, Lexer::LexList& list)
{
    switch (list.front().second)
    {
        case Lexer::Token::Q :
        {
            QuantedTerm::QType type;
            if (list.front().first == QuantedTerm::word[QuantedTerm::QType::EXISTS])
                type = QuantedTerm::QType::EXISTS;
            else
                type = QuantedTerm::QType::FORALL;
            list.pop_front();

            if (list.front().second != Lexer::Token::V)
                return nullptr;
            Variable var = reas.getV(list.front().first);
            list.pop_front();

            Terms* term = parseTerm(reas, list);
            if (term == nullptr)
                return nullptr;
            QuantedTerm* qterm = new QuantedTerm(type, var, term);
            delete term;
            return qterm;
        }
        case Lexer::Token::S :
        {
            Symbol s = reas.getS(list.front().first);
            list.pop_front();

            if (list.front().second != Lexer::Token::lb)
                return nullptr;
            list.pop_front();

            std::list<Terms*> terms;
            while (true)
            {
                if (list.front().second == Lexer::Token::rb)
                {
                    list.pop_front();
                    break;
                }
                Terms* nextT = parseTerm(reas, list);
                if (nextT == nullptr)
                    return nullptr;
                terms.push_back(nextT);

                if (list.front().second == Lexer::Token::c)
                    list.pop_front();
                else if (list.front().second == Lexer::Token::rb)
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
        case Lexer::Token::V:
        {
            Variable* var = reas.getV(list.front().first).clone();
            list.pop_front();
            return var;
        }
        case Lexer::Token::lb:
        {
            list.pop_front();
            return parseTerm(reas, list);
        }
        default:
            return nullptr;
    }
}

void addStatement(Reasoning& reas, std::string source)
{
    registerVars(reas, source);
//std::cerr<<source<<std::endl;
    Lexer lex(reas);
//for (auto w : lex.words)
//std::cerr<<'['<<w.first<<"] ";
//std::cerr<<std::endl;
    lex.recognize(source);
    /*for (auto ll : lex.lastResult)
    {
        for (auto l : ll)
            std::cout<<'['<<Lexer::tokToStr(l.second)<<"]:"<<l.first<<std::endl;
        std::cout<<std::endl;
    }*/
    Terms* parsed = nullptr;
    for (auto r : lex.lastResult)
    {
        parsed = parseTerm(reas, r);
        if (parsed != nullptr)
            break;
    }
    reas.addSub(parsed);
}
