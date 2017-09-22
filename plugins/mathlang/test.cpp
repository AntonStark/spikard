#include <sstream>
#include "logic.hpp"
#include "signature.hpp"
#include "parser.hpp"

#include "../../json.hpp"
using json = nlohmann::json;

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

    Section theorem("Название раздела");
    theorem.defType("Logical");
    theorem.defVar("A", "Logical");
    theorem.defSym("\\Rightarrow ", {"Logical", "Logical"}, "Logical");
    theorem.defVar("B", "Logical");
    theorem.addAxiom("\\forall C\\typeof Logical \\Rightarrow (A, B)");
    theorem.doSpec("5", "5.1");
    theorem.doMP("(2)", "(6)");
    theorem.printB(cout);

    json j = theorem.toJson();
    cout << j.dump(2);
    cout << endl << endl << flush;
    HierarchyItem* rec = Section::fromJsonE(j);
    dynamic_cast<Section*>(rec)->printB(cout);

    return 0;
}
