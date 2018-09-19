//
// Created by anton on 20.06.18.
//

#include <string>

#include "../parser/lexer.hpp"

using namespace std;
using namespace Parser2;

int main() {
    Lexer lex;
    string brackets = R"(\Bigg(\bigg[)"
                           R"(x\mapsto \delta_1\sum_{k=1}^s)"
                           R"(\big(v_{l,1}+v_{l,2}+v_{l,3}-a^*_{k,1}-a^*_{k,2}-a^*_{k,3})"
                           R"(\big)\bigg],\\)"
                           R"(\delta_2\cdot\sum_{k=1}^s\sum_{l=1}^t)"
                           R"(det\Big(a_{k,i}(v_{l,j})\Big)\Bigg))";

    CurAnalysisData data = CurAnalysisData(lex, brackets);

    /*for (const auto& bI : data.bracketInfo) {
        for (auto i = bI.first; i < bI.second + 1; ++i)
            cout << data.getVal(data.lexems.at(i)) << ' ';
        cout << endl;
    }
    cout << "=======================================" << endl;*/
    for (const auto& L : data.layers) {
        for (const auto& l : L->getLexems())
            cout << lex.print(l) << " ";
        cout << "\t\t" << L->_excludes.size() << endl;
        for (auto bP : L->bracketPairs)
            cout << bP.first - L->_bounds.first << "," << bP.second - L->_bounds.first << "  ";
        cout << endl;
    }
    cout << flush;

    return 0;
}
