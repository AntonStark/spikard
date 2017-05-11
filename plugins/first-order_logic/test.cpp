#include <sstream>
#include "logic.hpp"
#include "signature.hpp"
#include "parser.hpp"
//#include "inference.hpp"

using namespace std;

/*Signature s({{">=",2}, {"=",2}, {"!=",2}, {"in",2}, {"!in",2}},
            {{"add",2}, {"mul",2}},
            {"0", "1"});
TermsFactory tf(s);
FormulasFactory ff;*/
//Signature s({{"!=", natural_mt, 2, logical_mt}});


/*inline FCard getS()
{
    string input;
    getline(cin, input);
    Lexer lex(s);
    Parser inter(lex, ff, tf, s, input);
    return *inter.stage3.begin();
}*/

/*void readLabel(Inference::FPath& fpath)
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
};*/

int main(void)
{
    /*Lexer lex(s);
    Parser inter(lex, ff, tf, s, "\\forall x >=(mul(x, x), 0) \\Rightarrow =(0,0)");
    FCard ax1 = *inter.stage3.begin();*/

    /*FCard ax1 = getF();
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
            deduce.check(deduce.getS(lA.first)->getSub(lA.second), flags);
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
    }*/
    MathType natural("Natural");
    MathType set("Set");

    Symbol n("\\lnot ", {logical_mt}, logical_mt); //можно и без {скобок}
    Symbol o({"\\lor ", {2, logical_mt}, logical_mt});
    Symbol a({"\\land ", {2, logical_mt}, logical_mt});
    Symbol r({"\\Rightarrow ", {2, logical_mt}, logical_mt});

    Variable x("x", natural);
    Variable one("1", natural);
//    Term equ(ne, {x, one});

    Reasoning reas;
    MathType group("Group");
    MathType real("Real");
    reas.addType(logical_mt);
    reas.addType(natural);
    reas.addType(group);
    reas.addType(real);

    Symbol gr({">", {real, real}, logical_mt});
    reas.addSym(r);
//    reas.addSym(gr);
    reas.addSym(">", {2, real}, logical_mt);
    Symbol ze({"0", {}, real});
    reas.addSym(ze);
    Symbol ne({"!=", {2, real}, logical_mt});
    reas.addSym(ne);
    addStatement(reas, "\\forall \\epsilon\\typeof Real \\Rightarrow (>(\\epsilon, 0()), \\exists N\\typeof Real !=(0(), N))");
    //todo меньше new/delete в термах, потом
    reas.print(cerr);


    cout<<flush;
    cerr<<flush;
    return 0;
}