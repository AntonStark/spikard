#include <sstream>
#include "logic.hpp"
#include "signature.hpp"
#include "formulas.hpp"
#include "parser.hpp"

using namespace std;

int main(void)
{
    Signature s({{">=",2}, {"=",2}, {"!=",2}, {"in",2}, {"!in",2}}, {{"add",2}, {"mul",2}}, {"0", "1"});
    TermsFactory tf(s);

    Function* f1 = s.getF("add");
    Function* o(s.getF("1"));
    Function* one = s.getF("1");
//    Term t1(&f1, {&x, &o});

    Function* f2 = s.getF("mul");
//    Term t2(&f2, {&t1, &t1});

    Function* n = s.getF("0");
    Term* ni = tf.makeTerm(n, {});
    Predicate* p1 = s.getP(">=");

    Variable* x = tf.makeVar("x");
    Variable* y = tf.makeVar("y");
    Term* te = tf.makeTerm(f1, {x, ni});
    Term* te2 = tf.makeTerm(f2, {te, y});
    Term* ty = tf.makeTerm(f2, {te, y});
    bool eq = (te2 == ty);

    Lexer lex(s);
    FormulasFactory ff;
    Parser inter(lex, ff, tf, s, "\\forall x >=(mul(x, x), 0) \\Rightarrow =(0,0)");
    (*inter.stage3.begin())->print();

    cout<<flush;cerr<<flush;
    return 0;
}