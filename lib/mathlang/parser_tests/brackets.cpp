//
// Created by anton on 20.06.18.
//

#include <string>

#include "../parser2.hpp"

using namespace std;
using namespace Parser2;

int main() {
    std::string brackets = R"(\Bigg(\bigg[)"
                           R"(x\mapsto \delta_1\sum_{k=1}^s)"
                           R"(\big(v_{l,1}+v_{l,2}+v_{l,3}-a^*_{k,1}-a^*_{k,2}-a^*_{k,3})"
                           R"(\big)\bigg],\\)"
                           R"(\delta_2\cdot\sum_{k=1}^s\sum_{l=1}^t)"
                           R"(det\Big(a_{k,i}(v_{l,j})\Big)\Bigg))";

    Parser2::CurAnalysisData data = CurAnalysisData(brackets);

    for (const auto& bI : data.bracketInfo) {
        for (auto i = bI.first; i < bI.second + 1; ++i)
            cout << data.getVal(data.lexems.at(i)) << ' ';
        cout << endl;
    }
    cout << "=======================================" << endl;
    for (const auto& L : data.layers) {
        for (const auto& l : L->lexems)
            std::cout << data.getVal(l) << " ";
        std::cout << "\t\t" << L->_placeholders << std::endl << std::flush;
    }

    return 0;
}
