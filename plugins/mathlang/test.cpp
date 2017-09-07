#include <sstream>
#include "logic.hpp"
#include "signature.hpp"
#include "parser.hpp"

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

    Reasoning reas;
    MathType group("Group");
    MathType real("Real");
    Variable x("x", real);
    Variable one("1", real);



    /*Symbol gr({">", {real, real}, logical_mt});
    Term t(gr, {&x, &one});
    const Terms* sdf = new ForallTerm(x, &t);
    std::cerr << "sdf is ForalTerm: " << (dynamic_cast<const ForallTerm*>(sdf) != nullptr) << std::endl;*/

//    reas.addSym(gr);
    Symbol ze({"0", {}, real});
    Symbol ne({"!=", {2, real}, logical_mt});
//    addStatement(reas, "\\forall \\epsilon\\typeof Real \\Rightarrow (>(\\epsilon, 0()), \\exists N\\typeof Real !=(0(), N))");
    //todo меньше new/delete в термах, потом
//    reas.print(cerr);


    /*Reasoning logic;
    logic.addType("Logical");
    logic.addSym({n, o, a, r});
    addStatement(logic, "\\forall A\\typeof Logical \\forall B\\typeof Logical \\Rightarrow (A, \\Rightarrow (B, A))");
    logic.addVar("A", logic.getT("Logical"));
    logic.addVar("B", logic.getT("Logical"));
    addStatement(logic, "\\Rightarrow (A, \\Rightarrow (B, A))");
//    logic.printNamespace();
    logic.get({1})->print();
//    logic.get({1})->printNamespace();
    addStatement(logic, "A");
    logic.deduceMP({3}, {2});
    logic.print();

    addStatement(logic, "B");
    logic.deduceMP({5}, {4});
//    logic.print(std::cerr);
    Term b(ne, {&x, &one});
    logic.deduceSpec({1}, {5});
    logic.deduceSpec({1}, {2}, {3});*/
//    logic.print();
    /*std::cout << "*arg(2): " << *dynamic_cast<const Term*>(logic.getTerms({1}))->arg(2) <<
            "; dyncast<FT>: " << (dynamic_cast<const ForallTerm*>(dynamic_cast<const Term*>(logic.getTerms({1}))->arg(2))!= nullptr) << std::endl;
*/

    Section theorem;
    theorem.pushDefType("Logical");
    theorem.pushDefVar("A", "Logical");
    theorem.pushDefSym("\\Rightarrow ", {"Logical", "Logical"}, "Logical");
    theorem.pushDefVar("B", "Logical");
    theorem.pushAxiom("\\Rightarrow (A, B)");

    cout<<flush;
    cerr<<flush;
    return 0;
}