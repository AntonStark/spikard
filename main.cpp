#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <termios.h>
#include <dlfcn.h>
#include <vector>
#include <map>

#include "core.h"
//#include "core.cpp"

using namespace std;

void printHelloMsg();
void echo_off();
void echo_on();
string myHash(string);
  void dlTest();
void parseComand(string, string&, vector<string>&);

static struct termios stored_settings;

/*void testing(vector<string> sd) {
    for (auto it = sd.begin(); it != sd.end(); it++)
        cout<<*it;
    return;
}*/   

int main() {
    printHelloMsg();
    Core core("comands.list");
    while (true) {
        cout<<core.user()<<">: ";
        string line, cmdName;
        vector<string> cmdArgs;
        getline(cin, line);
        parseComand(line, cmdName, cmdArgs);
        try 
        {
            core.call(cmdName, cmdArgs);
        }
        catch (exit_ex)
        {
            break;
        }
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
