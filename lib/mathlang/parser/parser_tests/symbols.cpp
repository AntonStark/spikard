//
// Created by anton on 03.09.18.
//


#include <string>
#include <iostream>

#include "../lexer.hpp"

using namespace std;
using namespace Parser2;

CurAnalysisData e2eTest(Lexer& lex, string input) {
    auto data = lex.recognize(input);
    if (data.res.success)
        cout << "OK" << endl;
    else
        cerr << data.res.at << ": " << data.res.mess << endl;
    return data;
};

int main () {
    Lexer& lex = texLexer;
    string source = R"(\Theta_{l,m_l}(\theta)=N_{l,m_l}P_l^{m_l}(\cos\theta))";
    auto data = e2eTest(texLexer, source);

    for (const auto& L : data.layers) {
        for (const auto& l : L->getLexems())
            cout << lex.print(l) << " ";
        cout << "\t\t" << L->_excludes.size() << endl;
        for (auto bP : L->bracketPairs)
            cout << bP.first - L->_bounds.first << "," << bP.second - L->_bounds.first << "  ";
        cout << endl;
    }
    cout << flush;

    // todo негативные тесты
    
    return 0;
}