#include <sstream>
#include "logic.hpp"
#include "signature.hpp"

using namespace std;

int main(void)
{
    Function f1("add", 2);
    Variable x("x");
    Constant o("1");
    Term t1(&f1, {&x, &o});

    Function f2("mul", 2);
    Term t2(&f2, {&t1, &t1});

    Constant n("0");
    Predicate p1(">=", 2);
    Atom a(&p1, {&t2, &n});

    Quantifier q(Quantifier::QType::FORALL, x);
    Formula fo(q, a);

    string buf = "\\forall x^{~} ";
    Predicate eq("=", 2);
    Atom fo2(&eq, {&x, &o});

    LOperation l(LOperation::LType::AND);
    Formula fa(l, fo, fo2);
    stringstream ss;
    fo.print(ss); ss << endl;
    fa.print(ss); ss << endl;

    cout << ss.str();

    cout << "add == mul? " << (f1 == f2) << endl;

    Signature signature({{">=",2}, {"=",2}, {"!=",2}, {"in",2}, {"!in",2}}, {{"add",2}, {"mul",2}}, {"0", "1"});
    Interpr inter(signature);

    Formula* formula = inter.interpretFormula("\\forall a (!=(a, 0)\\Rightarrow\\exists b(in(b,a)\\land\\exists c (in(c,b)\\Rightarrow!in(c,a))))").get();


    cout<<flush;cerr<<flush;
    return 0;
}