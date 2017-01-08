//
// Created by anton on 04.01.17.
//

#include "formulas.hpp"

class Signature::sym_doubling : public std::invalid_argument
{
public:
    sym_doubling(const std::string& symName)
            : std::invalid_argument("Попытка дублирования символа \"" + symName +"\".\n") {}
};
/*class Signature::sym_exists : public std::invalid_argument
{
public:
    sym_exists(const std::string& symName)
            : std::invalid_argument("Символ с именем \"" + symName + "\" уже определён.\n") {}
};*/
class Signature::no_sym : public std::invalid_argument
{
public:
    no_sym(const std::string& symName)
            : std::invalid_argument("Cимвол \"" + symName + "\" не определён.\n") {}
};

Signature::Signature(std::list<std::pair<std::string, unsigned> > _R,
                     std::list<std::pair<std::string, unsigned> > _F,
                     std::list<std::string> _C)
{
    /*//Проверка, что в списках нет повторяющихся имён
    std::map<std::string, unsigned> tR;
    std::map<std::string, unsigned> tF;
    std::set<std::string> tC;
    for (auto r : _R)
    {
        if (tR.find(r.first) != tR.end())
            throw sym_doubling(r.first);
        else
            tR.insert(r);
    }
    for (auto f : _F)
    {
        if (tF.find(f.first) != tF.end())
            throw sym_doubling(f.first);
        else
            tF.insert(f);
    }
    for (auto c : _C)
    {
        if (tC.find(c) != tC.end())
            throw sym_doubling(c);
        else
            tC.insert(c);
    }

    //Првоерка, что задаваемые множества не будут пересекаться по именам
    for (auto f : tF)
        if (tR.find(f.first) != tR.end())
            throw sym_doubling(f.first);
    for (auto c : tC)
        if (tR.find(c) != tR.end())
            throw sym_doubling(c);
    for (auto c : tC)
        if (tF.find(c) != tF.end())
            throw sym_doubling(c);

    //Инициализация множеств
    for (auto r : tR)
        R[r.first] = (new Predicate(r.first, r.second) );
    for (auto f : tF)
        F[f.first] = (new Function(f.first, f.second) );
    for (auto c : tC)
        C[c] = (new Constant(c) );*/

    for (auto r : _R)
        addP(r.first, r.second);
    for (auto f : _F)
        addF(f.first, f.second);
    for (auto c : _C)
        addC(c);
}

Signature::nameT Signature::checkName(const std::string& name) const
{
    if (isPredName(name))
        return nameT::predicate;
    if (isFuncName(name))
        return nameT::function;
    if (isConsName(name))
        return nameT::constant;
    return nameT::none;
}

unsigned Signature::arity(const std::string& name) const
{
    if (isPredName(name))
    { return R.at(name)->getArity(); }
    else if (isFuncName(name))
    { return F.at(name)->getArity(); }
    else
    { return static_cast<unsigned>(-1);}
}

void Signature::addP(const std::string& name, unsigned arity)
{
    if (checkName(name) != nameT::none)
        throw sym_doubling(name);
    R[name] = std::shared_ptr<Predicate>(new Predicate(name, arity) );
}
void Signature::addF(const std::string& name, unsigned arity)
{
    if (checkName(name) != nameT::none)
        throw sym_doubling(name);
    F[name] = std::shared_ptr<Function>(new Function(name, arity) );
}
void Signature::addC(const std::string& name)
{
    if (checkName(name) != nameT::none)
        throw sym_doubling(name);
    C[name] = std::shared_ptr<Constant>(new Constant(name) );
}

std::shared_ptr<Predicate> Signature::getP(const std::string& name) const
{
    if (isPredName(name))
        return R.at(name);
    else
        throw no_sym(name);
}
std::shared_ptr<Function> Signature::getF(const std::string& name) const
{
    if (isFuncName(name))
        return F.at(name);
    else
        throw no_sym(name);
}
std::shared_ptr<Constant> Signature::getC(const std::string& name) const
{
    if (isConsName(name))
        return C.at(name);
    else
        throw no_sym(name);
}

unsigned long Signature::maxLength(nameT type) const
{
    unsigned long maxLen = 0;
    switch (type)
    {
        case nameT::predicate:
        {
            for (auto r : R)
                if (r.first.length() > maxLen)
                    maxLen = r.first.length();
            break;
        }
        case nameT::function:
        {
            for (auto f : F)
                if (f.first.length() > maxLen)
                    maxLen = f.first.length();
            break;
        }
        case nameT::constant:
        {
            for (auto c : C)
                if (c.first.length() > maxLen)
                    maxLen = c.first.length();
            break;
        }
        case nameT::none: {}
    }
    return maxLen;
}

/*
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
bool splitByTopLevelLO(std::string source, std::string& left, LOperation::LType& oper, std::string& right)
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
        oper = LOperation::LType::THAN;
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
            oper = LOperation::LType::AND;
            left = source.substr(0, i);
            right = source.substr(i+5);
        }
        else
        {
            oper = LOperation::LType::OR;
            left = source.substr(0, j);
            right = source.substr(j+4);
        }
        return true;
    }
}


bool Interpr::checkForAtom(const std::string& text, std::string& pred, std::string& paren)
{
    size_t i = text.find('(');
    size_t j = findPairBracket(text, i);
    pred = text.substr(0, i);
    paren = text.substr(i);
    stripBrackets(paren);
    return ((i != 0) && (j == text.length()) && sigma.isPredName(pred));
}
bool Interpr::checkFBegin(const std::string& foText)
{
    return (checkForQuant(foText) || checkForNOT(foText) ||
            (foText.front() == '(') || (foText.front() == ')') || checkPred(foText).length());
}
bool Interpr::checkForNOT(const std::string& foText)
{ return (foText.compare(0, 5, "\\lnot") == 0); }
bool Interpr::checkForQuant(const std::string& foText)
{
    return (foText.compare(0, 7, "\\forall") == 0 ||
            foText.compare(0, 7, "\\exists") == 0);
}
std::string Interpr::checkPred(const std::string& source)
{
    unsigned long maxLen = std::min(sigma.maxLength(Signature::nameT::predicate),
                                    source.length());
    for (unsigned long i = 1; i <= maxLen; ++i)
        if (sigma.isPredName(source.substr(0, i)))
            return source.substr(0, i);
    return "";
}
std::string Interpr::checkFunc(const std::string& source)
{
    unsigned long maxLen = std::min(sigma.maxLength(Signature::nameT::function),
                                    source.length());
    for (unsigned long i = 1; i <= maxLen; ++i)
        if (sigma.isFuncName(source.substr(0, i)))
            return source.substr(0, i);
    return "";
}
std::string Interpr::checkCons(const std::string& source)
{
    unsigned long maxLen = std::min(sigma.maxLength(Signature::nameT::constant),
                                    source.length());
    for (unsigned long i = 1; i <= maxLen; ++i)
        if (sigma.isConsName(source.substr(0, i)))
            return source.substr(0, i);
    return "";
}


std::shared_ptr<LOperation> Interpr::makeLOperation(const std::string& text)
{
    LOperation::LType type;
    if (text.compare(0, 5, "\\lnot") == 0)
    { type = LOperation::LType::NOT; }
    else if (text.compare(0, 5, "\\land") == 0)
    { type = LOperation::LType::AND; }
    else if (text.compare(0, 4, "\\lor") == 0)
    { type = LOperation::LType::OR; }
    else if (text.compare(0, 11, "\\Rightarrow") == 0)
    { type = LOperation::LType::THAN; }
    else
        throw std::invalid_argument("Ошибка в LOperation(str) [str = " + text + "]\n");
    return std::make_shared<LOperation>(type);
}
std::shared_ptr<Quantifier> Interpr::makeQuantifier(const std::string& text)
{
    Quantifier::QType type;
    if (text.compare(0, 7, "\\forall") == 0)
    { type = Quantifier::QType::FORALL; }
    else if (text.compare(0, 7, "\\exists") == 0)
    { type = Quantifier::QType::EXISTS; }
    else
        throw std::invalid_argument("Ошибка в Quantifier(str) [str = " + text + "]\n");

    std::string temp = text.substr(7);
    prepareForName(temp);
    Variable arg(temp);
    return std::make_shared<Quantifier>(type, arg);
}
std::shared_ptr<Modifier> Interpr::makeModifier(const std::string& text)
{
    if (checkForQuant(text))
    { return std::shared_ptr<Quantifier>(makeQuantifier(text)); }
    else if (checkForNOT(text))
    { return std::shared_ptr<LOperation>(makeLOperation(text)); }
    else
    { throw std::invalid_argument("Ошибка в makeModifier(str) [str = " + text + "]\n");}
}
inline std::shared_ptr<Formulas> Interpr::makeFormulas(const std::string& source)
{
    std::string pred, paren;
    if (checkForAtom(source, pred, paren))
        return std::make_shared<Atom>(sigma.getP(pred),
                                      getTermsFromParen(paren));
    else
        return std::shared_ptr<Formula>(interpretFormula(source));
}

std::list<std::shared_ptr<Terms> > Interpr::getTermsFromParen(const std::string& paren)
{
    std::list<std::string> sargs;
    std::list<std::shared_ptr<Terms> > args;
    size_t i = 0, j;
    do
    {
        j = paren.find(',', i);
        sargs.push_back(paren.substr(i, j-i));
        i = j+1;
    }
    while (j != paren.npos);

    for (auto l : sargs)
    {
        prepareForName(l);
        if (l.find('(') != l.npos)
        {
            std::string func, pparen;
            func = l.substr(0, l.find('('));
            pparen = l.substr(l.find('('));
            if (!sigma.isFuncName(func))
                throw std::invalid_argument("Неизвестный функциональный символ: " + func + ".\n");
            args.push_back(std::make_shared<Term>(sigma.getF(func),
                                                  getTermsFromParen(pparen) ));
        }
        else if (sigma.isConsName(l))
            args.push_back(std::make_shared<Constant>(l));
        else
            args.push_back(std::make_shared<Variable>(l));
    }
    return args;
}
std::shared_ptr<Formula> Interpr::interpretFormula(std::string foText)
{
    std::shared_ptr<Modifier> mod;
    std::shared_ptr<Formulas> arg1, arg2;
    std::cerr << foText <<std::endl;
    stripBrackets(foText);
    if (checkForQuant(foText) || checkForNOT(foText))
    {
        ///Если начинается с квантора (или 'не'), отступим на один символ и поищем
        ///начало ближайшей подформулы. То, что было до - квантор+переменная или 'не'
        unsigned long i = 1;
        while (i < foText.length() && !checkFBegin(foText.substr(i)) )
            ++i;
        if (!checkFBegin(foText.substr(i)))
            throw std::invalid_argument("Неверно построенная формула: " + foText + ".\n");
        mod = std::shared_ptr<Modifier>(makeModifier(foText.substr(0, i)) );
        arg1 = makeFormulas(foText.substr(i));
        return std::make_shared<Formula>(mod, arg1);
    }
    else
    {
        //ищем top-level LOpperation
        std::string left, right;
        LOperation::LType oper;
        if (splitByTopLevelLO(foText, left, oper, right))
        {
            mod = std::make_shared<LOperation>(oper);
            arg1 = makeFormulas(left);
            arg2 = makeFormulas(right);
            return std::make_shared<Formula>(mod, arg1, arg2);
        }
        else
            throw std::invalid_argument("Ошибка в Formula(str) [str = " + foText + "]\n");
    }
}*/
