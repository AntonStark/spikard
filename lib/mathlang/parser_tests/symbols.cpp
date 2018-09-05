//
// Created by anton on 03.09.18.
//

#include <string>

#include "../lexer2.hpp"

using namespace std;
using namespace Parser2;

int main () {
    string source = R"(\left\{\ell_{ i}\right\})";
    auto data = CurAnalysisData(source);

    auto layer = *data.layers.begin();
    for (auto& sB : layer->symbolBounds) {
        LexemeSequence lS(next(layer->_base.begin(), sB.first),
                          next(layer->_base.begin(), sB.second));
        for (auto& l : lS)
            cout << data.getVal(l) << " ";
        cout << endl;
    }
    return 0;
}