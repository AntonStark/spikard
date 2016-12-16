#include <sstream>
#include "logic.hpp"

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
//    Predicate ad("add", 2);
//    cout << "f:add == p:add? " << (f1 == ad) << endl; //OK, сравнивать Function и Predicate нельзя
    return 0;
}