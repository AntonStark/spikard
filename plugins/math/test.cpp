#include <sstream>
#include <fstream>

#include "../../lib/mathlang/basics/primary.hpp"
#include "../../lib/mathlang/consepts/terms.hpp"
#include "../../lib/mathlang/proof/definition.hpp"
//#include "../../lib/mathlang/parser/old_parser.hpp"
#include "../../lib/mathlang/view.hpp"

#include "../../json.hpp"

using json = nlohmann::json;

using namespace std;

int main(void)
{
    PrimaryType natural("Natural");
    PrimaryType set("Set");

    /*Map n("\\lnot ", {&logical_mt}, &logical_mt); //можно и без {скобок}
    Map o({"\\lor ", {2, &logical_mt}, &logical_mt});
    Map a({"\\land ", {2, &logical_mt}, &logical_mt});
    Map r({"\\Rightarrow ", {2, &logical_mt}, &logical_mt});

    PrimaryType group("Group");
    PrimaryType real("Real");
    Variable x("x", &real);
    Variable one("1", &real);

    //todo  2) при обрыве работы происходит сохранение с именем autosave?


    Map gr({">", {&real, &real}, &logical_mt});
    Term t(gr, {&x, &one});
    const AbstractTerm* sdf = new ForallTerm(&x, &t);
//    std::cerr << "sdf is ForalTerm: " << (dynamic_cast<const ForallTerm*>(sdf) != nullptr) << std::endl;

    Map ze({"0", {}, &real});
    Map ne({"!=", {2, &real}, &logical_mt});*/
    //todo меньше new/delete в термах, потом

    BranchNode entry("global");
    entry.startLecture("Название раздела");
    Hierarchy* h = entry.getSub(1);
    if (auto* pn = static_cast<PrimaryNode*>(h)) {
        pn->defType("Logical");
        pn->defVar("A", "Logical");
        pn->defSym("\\Rightarrow ", {"Logical", "Logical"}, "Logical");
        pn->defVar("B", "Logical");
        pn->addTerm("\\forall C\\in Logical \\Rightarrow (A, B)");
        pn->doSpec("5", "5.1");
        pn->doMP("(2)", "(6)");

        /*cout << pn->print(new PlainText()) << endl;
        cout << pn->Node::print(new PlainText());

        cerr << pn->print(new AsJson()) << endl << flush;*/
    }
    stringstream ss;
    ss << entry.print(new AsJson()) << endl;

    json j = json::parse(ss.str());
//    cerr << j.dump(2) << endl;

    auto bn = BranchNode::fromJson(j.at(0).at(1));

    auto pr = new PlainText;
    cout << bn->print(pr) << endl;
    cout << bn->Node::print(pr);
    cout << bn->getSub(1)->Node::print(pr);



    /*json j = theorem.toJson();
    cout << j.dump(2);
    cout << endl << endl << flush;
    Hierarchy* rec = Lecture::fromJsonE(j);*/

    return 0;
}
