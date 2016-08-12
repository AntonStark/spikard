#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <termios.h>
#include <dlfcn.h>
#include <vector>
#include <map>
#include <initializer_list>

#include "core.h"
#include "core.cmds.cpp"

using namespace std;

void printHelloMsg();
void echo_off();
void echo_on();
string myHash(string);
  void dlTest();
void parseComand(string, string&, vector<string>&);

string Core::userName = "?";
bool Core::run = true;
static struct termios stored_settings;

Core::Core(string cmdFile) {
    ifstream cmdFStr(cmdFile.c_str());
    if (cmdFStr == NULL) {
        cout<<"Отсутствует конфиг-файл команд."<<endl;
        exit(1);//Вообще-то плохо
    }
    string bufC, bufM;
    auto fnNames = {emptyComand, logIn, logOut, diary,
                    end, plugIn, printListOfComands, change};
    auto it = fnNames.begin();
    auto et = fnNames.end();
    while (it != et) {
        getline(cmdFStr, bufC);
        connect.insert(make_pair(bufC, *it));
        getline(cmdFStr, bufM);
        mans.insert(make_pair(bufC, bufM));
        ++it;
    }
    cmdFStr.close();
}

Core::~Core() {
    /*map<string, BaseComand*>::iterator it = connect.begin();
    while (it != connect.end()) {
        delete it->second;
        it->second = NULL;
        ++it;
    }//наверно избыточно*/
    connect.clear();
}

int main() {
    printHelloMsg();
    Core core("comands.list");
    while (Core::run) {
        cout<<Core::userName<<">: ";
        string line;
        getline(cin, line);
        string cmdName;
        vector<string> cmdArgs;
        parseComand(line, cmdName, cmdArgs);
        map<string, void (*)(vector<string>)>::iterator it = core.connect.find(cmdName);
        if (it == core.connect.end())
            cout<<"Неизвестная команда."<<endl;
        else
            core.connect[cmdName](cmdArgs);
    }
    return 0;
}

void printHelloMsg() {
    cout<<"Это черновой вариант ядра программы для моделирования и изменения объектов."<<endl;
    cout<<"\tВерсия программы от 15 января '16.\n\tРеализована возможность логина (лог действий и дневник) и главное меню."<<endl;
    cout<<"Для вывода списка доступных команд введите <помощь>"<<endl;
    return;
}

void echo_off(void) {
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ECHO);
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void echo_on(void) {
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

string myHash(string source) {
    string hash(32, (char)0);
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < source.length(); ++j)
            hash[i] += (char)((int)source[j]*3^(j+i));
        hash[i] = (char)((unsigned int)hash[i]%95 + 32);
    }
    return hash;
}

void dlTest() {
    int arg;
    cin>>arg;
    void* testplugin_handler;
    testplugin_handler = dlopen("./plugins/testplugin.so", RTLD_LAZY);
    if (!testplugin_handler) {
        cout<<"dlopen() error: "<<dlerror()<<endl;
        exit(1);
    }
    void (*test)(int);
    *(void**)(&test) = dlsym(testplugin_handler, "test");
    if (test == NULL) {
        cout<<"Function not found."<<endl;
        exit(1);
    }
    (*test)(arg);
    dlclose(testplugin_handler);
    return;
}

void parseComand(string line, string& cmdName, vector<string>& cmdArgs) {
    int s = line.find(' ');
    if (s == string::npos) {
        cmdName = line;
        return;
    }
    else {
        cmdName = line.substr(0, s);
        line.erase(0, s+1);
        s = line.find(' ');
        while (s != string::npos) {
            cmdArgs.push_back(line.substr(0, s));
            line.erase(0, s+1);
            s = line.find(' ');
        }
        cmdArgs.push_back(line);
    }
    return;
}
