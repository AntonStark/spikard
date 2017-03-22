#include <sstream>
#include "logic.hpp"
#include "signature.hpp"
#include "formulas.hpp"
#include "parser.hpp"
#include "inference.hpp"

using namespace std;

Signature s({{">=",2}, {"=",2}, {"!=",2}, {"in",2}, {"!in",2}},
            {{"add",2}, {"mul",2}},
            {"0", "1"});
TermsFactory tf(s);
FormulasFactory ff;


inline FCard getF()
{
    string input;
    getline(cin, input);
    Lexer lex(s);
    Parser inter(lex, ff, tf, s, input);
    return *inter.stage3.begin();
}

void readLabel(Inference::FPath& fpath)
{
    char c;
    string num;
    bool done = false;
    while (true)
    {
        cin.get(c);
        if (c == ':')
            break;
        if (c == ' ' || c == '\n')
        {
            if (num.length() == 0)
                continue;
            done = true;
            break;
        }
        num += c;
    }
    fpath.first = atoi(num.c_str());
    stack<Formula::ArgTy> buf, path;
    while (!done)
    {
        cin.get(c);
        if (c == ' ' || c == '\n')
            break;
        if (c == 'l')
            buf.push(Formula::ArgTy::f);
        else
            buf.push(Formula::ArgTy::s);
    }
    while (buf.size() != 0)
    {
        path.push(buf.top());
        buf.pop();
    }
    fpath.second = path;
};

//todo СОСРЕДОТОЧЬСЯ!!! логика предикатов как одна из теорий, а не нечно особенное жёстко фиксированное в коде
//todo эта теория задаёт логический тип, используемый логиками первого порядка
int main(void)
{
    /*Lexer lex(s);
    Parser inter(lex, ff, tf, s, "\\forall x >=(mul(x, x), 0) \\Rightarrow =(0,0)");
    FCard ax1 = *inter.stage3.begin();*/

    FCard ax1 = getF();
    Theory th({ax1});
    Inference deduce(ff, th);
    deduce.print(Inference::PrintTy::INC);

    int tr;
    unsigned lB;
    Inference::FPath lA;
    while (true)
    {
        scanf("%d", &tr);
        if (tr == 0)
            break;
        readLabel(lA);
        if (tr == -1)
        {
            bool flags[10];
            deduce.check(deduce.getF(lA.first)->getSub(lA.second), flags);
            for (int i = 0; i < 10; ++i)
                if (flags[i])
                    cout << " (" << i+1 << ")";
            cout << endl;
        }
        else
        {
            scanf("%d", &lB);
            if (lB == 0)
                deduce.performUnary(lA, tr);
            else
                deduce.performBinary(lA, lB, tr);
            deduce.print(Inference::PrintTy::INC);
        }
    }

    return 0;
}