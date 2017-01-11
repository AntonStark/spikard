#include <sstream>
#include "logic.hpp"
#include "signature.hpp"
#include "formulas.hpp"

using namespace std;

int main(void)
{
    Signature s({{">=",2}, {"=",2}, {"!=",2}, {"in",2}, {"!in",2}}, {{"add",2}, {"mul",2}}, {"0", "1"});

    Function* f1 = s.getF("add");
//    Variable x("x");
    Constant* o(s.getC("1"));
    Constant* one = s.getC("1");
//    Term t1(&f1, {&x, &o});

    Function* f2 = s.getF("mul");
//    Term t2(&f2, {&t1, &t1});

    Constant* n = s.getC("0");
    Predicate* p1 = s.getP(">=");

    /*Atom a(&p1, {&t2, &n});

    Quantifier q(Quantifier::QType::FORALL, x);
    ComposedF fo(q, a);

    Predicate eq("=", 2);
    Atom fo2(&eq, {&x, &o});

    LOperation l(LOperation::LType::AND);
    ComposedF fa(l, fo, fo2);
    stringstream ss;
    fo.print(ss); ss << endl;
    fa.print(ss); ss << endl;

    cout << ss.str();

    cout << "add == mul? " << (f1 == f2) << endl;

    Interpr inter(signature);

    std::shared_ptr<ComposedF> formula = inter.interpretFormula("\\forall a (!=(a, 0)\\Rightarrow\\exists b(in(b,a)\\land\\exists c (in(c,b)\\Rightarrow!in(c,a))))");

    cout<<'\n';
    formula->print();
    cout<<'\n';*/
    s.termsStorage.addV("x");
    s.termsStorage.addV("y");
    Variable* x = s.termsStorage.getV("x");
    Variable* y = s.termsStorage.getV("y");
    Term* te = s.termsStorage.makeTerm(f1, {x, n});
    Term* te2 = s.termsStorage.makeTerm(f2, {te, y});
    Term* ty = s.termsStorage.makeTerm(f2, {te, y});
    bool eq = (te2 == ty);



    cout<<flush;cerr<<flush;
    return 0;
}