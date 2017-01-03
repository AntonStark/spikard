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




std::vector<std::shared_ptr<Terms> > getTermsFromParen(const std::string& paren, Signature& sigma)
{
    std::list<std::string> sargs;
    std::vector<std::shared_ptr<Terms> > args;
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
            if (!sigma.checkFuncName(func))
                throw std::invalid_argument("Неизвестный функциональный символ: " + func + ".\n");
            args.push_back(std::make_shared<Term>(std::make_pair(func, sigma.arity(func)),
                                                  getTermsFromParen(pparen, sigma)) );
        }
        else if (sigma.checkConsName(l))
            args.push_back(std::make_shared<Constant>(l));
        else
            args.push_back(std::make_shared<Variable>(l));
    }
    return args;
}
inline std::shared_ptr<Formulas> makeFormulas(const std::string& source, Signature& sigma)
{
    std::string pred, paren;
    if (checkForAtom(source, pred, paren, sigma))
        return std::make_shared<Atom>(std::make_pair(pred, sigma.arity(pred)),
                                      getTermsFromParen(paren, sigma));
    else
        return std::shared_ptr<Formula>(interpretFormula(source, sigma));
}
Formula* interpretFormula(std::string foText, Signature& sigma)
{
    std::shared_ptr<Modifier> mod;
    std::shared_ptr<Formulas> arg1, arg2;
    std::cerr << foText <<std::endl;
    stripBrackets(foText);
    if (Quantifier::checkForQuant(foText) || LOperation::checkForNOT(foText))
    {
        ///Если начинается с квантора (или 'не'), отступим на один символ и поищем
        ///начало ближайшей подформулы. То, что было до - квантор+переменная или 'не'
        unsigned long i = 1;
        while (i < foText.length() && !checkFBegin(foText.substr(i), sigma))
            ++i;
        if (!checkFBegin(foText.substr(i), sigma))
            throw std::invalid_argument("Неверно построенная формула: " + foText + ".\n");
        mod = std::shared_ptr<Modifier>(Modifier::makeModifier(foText.substr(0, i)) );
        arg1 = makeFormulas(foText.substr(i), sigma);
        return (new Formula(mod, arg1));
    }
    else
    {
        //ищем top-level LOpperation
        std::string left, right;
        LOperation::LType oper;
        if (splitByTopLevelLO(foText, left, oper, right))
        {
            mod = std::make_shared<LOperation>(oper);
            arg1 = makeFormulas(left, sigma);
            arg2 = makeFormulas(right, sigma);
            return (new Formula(mod, arg1, arg2));
        }
        else
            throw std::invalid_argument("Ошибка в Formula(str) [str = " + foText + "]\n");
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

bool checkForAtom(const std::string& text, std::string& pred, std::string& paren, Signature& sigma)
{
    size_t i = text.find('(');
    size_t j = findPairBracket(text, i);
    pred = text.substr(0, i);
    paren = text.substr(i);
    stripBrackets(paren);
    return ((i != 0) && (j == text.length()) && sigma.checkPredName(pred));
}
bool checkFBegin(const std::string& foText, Signature& sigma)
{
    return (Quantifier::checkForQuant(foText) || LOperation::checkForNOT(foText) ||
            (foText.front() == '(') || (foText.front() == ')') || sigma.checkPredNameAtBegin(foText).length());
}