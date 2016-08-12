#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <termios.h>
#include <dlfcn.h>
#include <vector>
#include <map>

#include "core.hpp"

using namespace std;

void printHelloMsg();
void parseComand(string, string&, vector<string>&);

int main() {
    printHelloMsg();
    try
    {
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
            catch (no_fun_ex)
            {
                cout<<"Неизвестная команда."<<endl;
            }
        }
    }
    catch (no_cfg_file)
    {
        cout<<"Отстутствует конфиг-файл команд \"comands.list\"."<<endl;
        return 1;
    }
    return 0;
}

void printHelloMsg() {
    cout<<"Это черновой вариант ядра программы для моделирования и изменения объектов."<<endl;
    cout<<"\tВерсия программы от 26 января '16.\n\tРеализована возможность логина (лог действий и дневник) и главное меню."<<endl;
    cout<<"Для вывода списка доступных команд введите <помощь>"<<endl;
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
