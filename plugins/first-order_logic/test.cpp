#include <sstream>
#include "logic.hpp"
#include "signature.hpp"

using namespace std;

int main(void)
{
    Signature s({{">=",2}, {"=",2}, {"!=",2}, {"in",2}, {"!in",2}}, {{"add",2}, {"mul",2}}, {"0", "1"});

    Function::sh_p f1 = s.getF("add");
    Variable x("x");
    Constant::sh_p o = s.getC("1");
    Constant::sh_p one = s.getC("1");
//    Term t1(&f1, {&x, &o});

    Function::sh_p f2 = s.getF("mul");
//    Term t2(&f2, {&t1, &t1});

    Constant::sh_p n = s.getC("0");
    Predicate::sh_p p1 = s.getP(">=");


    /*Atom a(&p1, {&t2, &n});

    Quantifier q(Quantifier::QType::FORALL, x);
    Formula fo(q, a);

    Predicate eq("=", 2);
    Atom fo2(&eq, {&x, &o});

    LOperation l(LOperation::LType::AND);
    Formula fa(l, fo, fo2);
    stringstream ss;
    fo.print(ss); ss << endl;
    fa.print(ss); ss << endl;

    cout << ss.str();

    cout << "add == mul? " << (f1 == f2) << endl;

    Interpr inter(signature);

    std::shared_ptr<Formula> formula = inter.interpretFormula("\\forall a (!=(a, 0)\\Rightarrow\\exists b(in(b,a)\\land\\exists c (in(c,b)\\Rightarrow!in(c,a))))");

    cout<<'\n';
    formula->print();
    cout<<'\n';*/

    cout<<flush;cerr<<flush;
    return 0;
}