#include <sstream>
#include "logic.hpp"
#include "signature.hpp"
#include "parser.hpp"

using namespace std;

int main(void)
{
    MathType natural("Natural");
    MathType set("Set");

    Symbol n("\\lnot ", {logical_mt}, logical_mt); //можно и без {скобок}
    Symbol o({"\\lor ", {2, logical_mt}, logical_mt});
    Symbol a({"\\land ", {2, logical_mt}, logical_mt});
    Symbol r({"\\Rightarrow ", {2, logical_mt}, logical_mt});

    MathType group("Group");
    MathType real("Real");
    Variable x("x", real);
    Variable one("1", real);



    /*Symbol gr({">", {real, real}, logical_mt});
    Term t(gr, {&x, &one});
    const Terms* sdf = new ForallTerm(x, &t);
    std::cerr << "sdf is ForalTerm: " << (dynamic_cast<const ForallTerm*>(sdf) != nullptr) << std::endl;*/

    Symbol ze({"0", {}, real});
    Symbol ne({"!=", {2, real}, logical_mt});
    //todo меньше new/delete в термах, потом

    Section theorem;
    theorem.pushDefType("Logical");
    theorem.pushDefVar("A", "Logical");
    theorem.pushDefSym("\\Rightarrow ", {"Logical", "Logical"}, "Logical");
    theorem.pushDefVar("B", "Logical");
    theorem.pushAxiom("\\Rightarrow (A, B)");
    theorem.doMP("(2)", "(5)");

    cout<<flush;
    cerr<<flush;
    return 0;
}