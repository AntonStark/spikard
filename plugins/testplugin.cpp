#include <iostream>
using namespace std;

extern "C" void test(int t) {
    cout<<"Вызов функции\n\tдинамической библиотеки\n\tс параметром "<<t<<'.'<<endl;
    return;
}
