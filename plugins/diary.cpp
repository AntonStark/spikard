#include "./../core.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <initializer_list>

using namespace std;

void print(string some) {
    cout<<some<<endl;
}

class DiaryPlugin : public BaseModule {
private:
    void test(vector<string> cmdArgs) {
        cout<<testValue<<endl;
        for_each(cmdArgs.begin(), cmdArgs.end(), print);
    }
    int testValue;
    
    TermMap<DiaryPlugin> interface;
    BaseModule* parent;
    list<BaseModule*> modules;
public:
    DiaryPlugin(string, BaseModule*);
    ~DiaryPlugin();

    void call(string, vector<string>);
};

DiaryPlugin::DiaryPlugin(string cmdFile, BaseModule* _parent) {
    ifstream cmdFStr(cmdFile.c_str());
    if (cmdFStr == NULL) {
        cout<<"Отсутствует конфиг файл команд."<<endl;
        exit(1);
    }
    string bufC, bufM;
    auto fnNames = {&DiaryPlugin::test};
    auto it = fnNames.begin();
    auto et = fnNames.end();
    while (it != et) {
        getline(cmdFStr, bufC);
        getline(cmdFStr, bufM);
        interface.add(bufC, *it, bufM);
        ++it;
    }
    cmdFStr.close();
    
    testValue = 5;

    parent = _parent;
    modules.push_front(this);
}

DiaryPlugin::~DiaryPlugin() {
    while (modules.size() > 1)
        delete modules.back();
}

void DiaryPlugin::call(string cmdName, vector<string> cmdArgs) {
    auto it = interface.connect.find(cmdName);
    if (it == interface.connect.end())
        cout<<"Неизвестная команда."<<endl;
    else
        (this->*this->interface.connect[cmdName])(cmdArgs);
    return;
}
