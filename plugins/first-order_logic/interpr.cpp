/*
//
// Created by anton on 03.01.17.
//

#include "signature.hpp"

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
