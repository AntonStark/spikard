//
// Created by anton on 14.01.17.
//

#include "parser.hpp"

bool matchIndented(const std::string& source, const size_t indent, const std::string& word) {
    if (indent + word.length() > source.length())
        return false;
    else
        return (source.substr(indent, word.length()).compare(word) == 0);
}

std::string Lexer::tokToStr(const Token& tok) {
    switch (tok) {
        case Token::S : return "S";
        case Token::V : return "V";
        case Token::C : return "C";
        case Token::Q : return "Q";
            /*case Token::T : return "T";
            case Token::to: return "to";*/
        case Token::c : return "c";
        case Token::s : return "s";
        case Token::lb: return "lb";
        case Token::rb: return "rb";
    }
}

Lexer::Lexer(PrimaryNode* _where) : where(_where) {
    for (auto& s : where->index().getNames(NameTy::SYM))
        words[s] = Token::S;
    for (auto s : where->index().getNames(NameTy::VAR))
        words[s] = Token::V;
    for (auto s : where->index().getNames(NameTy::CONST))
        words[s] = Token::C;

    words[ Term::qword[Term::QType::FORALL] ] = Token::Q;
    words[ Term::qword[Term::QType::EXISTS] ] = Token::Q;
    words[","] = Token::c;  words[" "] = Token::s;
    words["("] = Token::lb; words[")"] = Token::rb;
}

std::string Lexer::recoverLexList(const LexList& list) {
    std::stringstream buf;
    for (const auto& l : list)
        buf << l.val;
    return buf.str();
}

PrimaryMT* parseType(const NameSpaceIndex& names, const std::string& source, unsigned& len) {
    unsigned typeNameLen = 1;
    while (!names.isThatType(source.substr(0, typeNameLen), NameTy::MT))
        if (typeNameLen == source.length())
            throw std::invalid_argument("Не найдено имя типа - неверный формат.\n");
        else
            ++typeNameLen;
    std::string typeName = source.substr(0, typeNameLen);
    len = typeNameLen;
    return getType(names, typeName);
}
// Из выражения вида "*\typeof <typeName>"
void Lexer::registerVar(std::string& source, unsigned indent) {
    unsigned nameLen = 1;
    while (!matchIndented(source, indent+nameLen, "\\in "))
        if (indent+nameLen == source.length())
            throw std::invalid_argument("Отсутствует \"\\\\in \" - неверный формат.\n");
        else
            ++nameLen;
    std::string name = source.substr(indent, nameLen);
    unsigned typeNameLen;
    PrimaryMT* type = parseType(where->index(), source.substr(indent + nameLen + 4), typeNameLen);
    where->defVar(name, type->getName());
    source.erase(indent + nameLen, 4+typeNameLen);
}
void Lexer::registerVars(std::string& source) {
    for (unsigned i = 0; i < source.length(); ++i)
        if (matchIndented(source, i, "\\forall ") ||
            matchIndented(source, i, "\\exists "))
            registerVar(source, i + 8);
}

void Lexer::refreshWords(NameTy type) {
    Token tok;
    switch (type) {
        case NameTy::VAR :
        { tok = Token::V; break; }
        case NameTy::CONST :
        { tok = Token::C; break; }
        case NameTy::SYM :
        { tok = Token::S; break; }
        case NameTy::MT :
        { /*tok = Token::T; break;*/return; }
    }

    std::set<std::string> buf = where->index().getNames(type);
    // Сначала убираем имена, которые есть и не нужны.
    auto e = words.end();
    auto be = buf.end();
    for (auto it = words.begin(); it != e; ++it)
        if (it->second == tok) {    //Имя есть
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

inline bool termBegin(const Lexer::Token& tok)
{ return (tok == Lexer::Token::Q || tok == Lexer::Token::lb ||
          tok == Lexer::Token::S || tok == Lexer::Token::V || tok == Lexer::Token::C); }

inline bool rb_c(const Lexer::Token& tok)
{ return (tok == Lexer::Token::rb || tok == Lexer::Token::c); }

// 1) возможно, через проверку принадлежности multimap<one, two> будет лаконичнее
// 2) ещё можно сделать set<pair<Token, Token> > и проверять принадлежность
// 3) или map<Token, set<Token> >
bool mayFollow(const Lexer::Token& one, const Lexer::Token& two) {
    typedef Lexer::Token Token;
    if (two == Lexer::Token::s)
        return true;
    switch (one) {
        case Token::S :
            return termBegin(two);
        case Token::V :
            return (rb_c(two) || termBegin(two));
        case Token::C :
            return (rb_c(two) || two == Token::S);
        /*case Token::T :
            return false;*/
        case Token::Q :
            return (two == Token::V);
        case Token::lb :
            return (two == Token::rb || termBegin(two));
        case Token::rb :
            return (rb_c(two) || two == Token::S);
        case Token::c :
            return (two == Token::V || two == Token::C || two == Token::S);
        case Token::s :
            return true;
    }
}

void delSpaces(Lexer::LexList& list) {
    auto e = list.end();
    for (auto it = list.begin(); it != e; ++it)
        if (it->tok == Lexer::Token::s)
            it = list.erase(it);
}

void Lexer::recognize(std::string source) {
    registerVars(source);
    refreshWords(NameTy::VAR);

    std::set<PartialResolved> partial;
    PartialResolved pair(0, {});
    partial.insert(pair);

    while (!partial.empty()) {
        pair = *partial.begin();
        partial.erase(partial.begin());
        if (pair.first == source.length()) {
            delSpaces(pair.second);
            lastResult.insert(pair.second);
            continue;
        }
        for (auto w : words) {
            if ( (pair.second.empty() || mayFollow(pair.second.back().tok, w.second))
                 //Если pair.second ещё пуст, то условие следования выполнено автоматически
                  && matchIndented(source, pair.first, w.first)
                ) {
                size_t newIndent = pair.first + w.first.length();
                LexList list = pair.second;
                list.push_back(w);
                partial.insert({newIndent, list});
            }
        }
    }
}

// QuantedTerm ::= Q V Term
Term* Lexer::parseQuantedTerm(Lexer::LexList& list) {
    typedef Lexer::Token Token;
    Term::QType type;
    if (list.front().val == Term::qword[Term::QType::FORALL])
        type = Term::QType::FORALL;
    else
        type = Term::QType::EXISTS;
    list.pop_front();

    if (list.front().tok != Token::V)
        return nullptr;
    Variable var = getVar(where->index(), list.front().val);
    list.pop_front();

    Terms* term = parseTerms(list);
    if (term == nullptr)
        return nullptr;
    Term* qterm = nullptr;
    try {
        if (type == Term::QType::FORALL)
            qterm = new ForallTerm(var, term);
        else
            qterm = new ExistsTerm(var, term);
    }
    catch (std::invalid_argument&) {}
    delete term;
    return qterm;
}

// Term ::= S lb [Term [c Term]] rb
// Term ::= Terms S Terms
Term* Lexer::parseTerm(Lexer::LexList& list) {
    typedef Lexer::Token Token;
    std::set<Map> syms = getSym(where->index(), list.front().val);
    list.pop_front();

    if (list.front().tok != Token::lb)
        return nullptr;
    list.pop_front();

    std::vector<Terms*> terms;
    while (true) {
        if (list.front().tok == Token::rb) {
            list.pop_front();
            break;
        }
        Terms* nextT = parseTerms(list);
        if (nextT == nullptr)
            return nullptr;
        terms.push_back(nextT);

        if (list.front().tok == Token::c)
            list.pop_front();
        else if (list.front().tok == Token::rb) {
            list.pop_front();
            break;
        }
        else
            return nullptr;
    }
    Term* tterm = nullptr;
    try { tterm = new Term(syms, terms); }
    catch (std::invalid_argument&) {}
    for (auto t : terms)
        delete t;
    return tterm;
}

Terms* Lexer::parseTerms(Lexer::LexList& list) {
    typedef Lexer::Token Token;
    Terms* parsed = nullptr;
    switch (list.front().tok) {
        case Token::Q : {
            parsed = parseQuantedTerm(list);
            break;
        }
        case Token::S : {
            parsed = parseTerm(list);
            break;
        }
        case Token::V : {
            parsed = getVar(where->index(), list.front().val).clone();
            list.pop_front();
            break;
        }
        case Token::C : {
            parsed = getConst(where->index(), list.front().val).clone();
            list.pop_front();
            break;
        }
        case Token::lb: {
            list.pop_front();
            parsed = parseTerms(list);
            list.pop_front();
            break;
        }
        default:
            return nullptr;
    }
    if (!list.empty() && list.begin()->tok == Token::S) {    // случай инфиксной записи бинарного терма
        std::set<Map> syms = getSym(where->index(), list.front().val);
        list.pop_front();
        Terms* secondOp = parseTerms(list);
        try { parsed = new Term(syms, {parsed, secondOp}); }
        catch (std::invalid_argument&) { parsed = nullptr; }
    }
    return parsed;
}

Terms* parse(PrimaryNode* where, std::string source) {
    Lexer lex(where);
    lex.recognize(source);

    Terms* parsed = nullptr;
    std::string tail;
    for (auto r : lex.lastResult) {
        parsed = lex.parseTerms(r);
        tail = Lexer::recoverLexList(r);
        if (parsed)
            break;
    }
    // todo выводить предупреждение о неоднозначности разбора,
    // если в итоге получится больше одного варианта...
    if (parsed)
        return parsed;
    throw std::invalid_argument("Не удалось построить терм по строке \"" + source + "\". "
                                "Остаток: " + tail + "\n");
}
