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
//    cout << t2.getName() << endl;

    Constant n("0");
    Predicate p1(">=", 2);
    Atom a(&p1, {&t2, &n});

    Quantifier q(Quantifier::QType::FORALL, &x);
    Formula fo(&q, &a);

    Predicate eq("=", 2);
    Atom fo2(&eq, {&x, &o});

    LOperation l(LOperation::LType::AND);
    Formula fa(&l, &fo, &fo2);

    stringstream ss;
    fo.print(ss); ss << endl;
    fa.print(ss); ss << endl;

    cout << ss.str();

    cout << "add == mul? " << (f1 == f2) << endl;

    Signature signature({{">=",2}, {"=",2}}, {{"add",2}, {"mul",2}}, {"0", "1"});
    cout<<flush;cerr<<flush;

//    Formula formula("\\forall a")
    return 0;
}