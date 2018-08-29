//
// Created by anton on 20.06.18.
//

#include <string>

#include "../parser2.hpp"

using namespace std;
using namespace Parser2;

/*void print(std::vector<Parser2::TexCommand> vec, size_t pl) {
    for (const auto& c : vec)
        std::cout << (c._cmd.empty() ? "." : c._cmd) << " ";
    std::cout << "\t\t" << pl << std::endl << std::flush;
}*/

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


/*
    for (const auto& l : data.layers)
        print(l->_cmds, l->_placeholders);
*/

    return 0;
}
