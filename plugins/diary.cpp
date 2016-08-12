#include "./../core.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <initializer_list>
#include <dlfcn.h>

using namespace std;

void print(string some) {
    cout<<some<<endl;
}
class SharedObject;
class DiaryPlugin : public BaseModule {
private:
    int testValue;
    void test(vector<string> cmdArgs) {
        cout<<testValue<<endl;
        for_each(cmdArgs.begin(), cmdArgs.end(), print);
    }
    //void diary(vector<string> cmdArgs);

    map<string, void (DiaryPlugin::*)(vector<string>)> connect;
    map<string, string> mans;
    void add(string cmd, void (DiaryPlugin::*pFun)(vector<string>), string man)
    {
        connect.insert(make_pair(cmd, pFun));
        mans.insert(make_pair(cmd, man));
        return;
    }
    SharedObject* fabric;
public:
    DiaryPlugin(string, BaseModule*, SharedObject*);
    ~DiaryPlugin() {}
    void destroy()
    {
        fabric->destroy(this);
        return;
    }

    bool ask(string, vector<string>);
};

//******функционал плагина********

/*void DiaryPlugin::diary(vector<string> cmdArgs)
{
    char buffer[60];
    time_t seconds = time(NULL);
    tm *timeinfo = localtime(&seconds);
    char format[] = "%Y, %H:%M:%S";
    strftime(buffer, 60, format, timeinfo);
    string date_time(buffer);
    switch (timeinfo->tm_mon) {
        case 0 : {
            date_time = " января " + date_time;
            break;
        }
        case 1 : {
            date_time = " февраля " + date_time;
            break;
        }
        case 2 : {
            date_time = " марта " + date_time;
            break;
        }
        case 3 : {
            date_time = " апреля " + date_time;
            break;
        }
        case 4 : {
            date_time = " мая " + date_time;
            break;
        }
        case 5 : {
            date_time = " июня " + date_time;
            break;
        }
        case 6 : {
            date_time = " июля " + date_time;
            break;
        }
        case 7 : {
            date_time = " августа " + date_time;
            break;
        }
        case 8 : {
            date_time = " сентября " + date_time;
            break;
        }
        case 9 : {
            date_time = " октября " + date_time;
            break;
        }
        case 10 : {
            date_time = " ноября " + date_time;
            break;
        }
        case 11 : {
            date_time = " декабря " + date_time;
            break;
        }
        default :
            break;
    }

    date_time = to_string(timeinfo->tm_mday) + date_time;

    switch (timeinfo->tm_wday) {
        case 1 : {
            date_time = "Понедельник, " + date_time;
            break;
            }
        case 2 : {
            date_time = "Вторник, " + date_time;
            break;
            }
        case 3 : {
            date_time = "Среда, " + date_time;
            break;
            }
        case 4 : {
            date_time = "Четверг, " + date_time;
            break;
            }
        case 5 : {
            date_time = "Пятница, " + date_time;
            break;
            }
        case 6 : {
            date_time = "Суббота, " + date_time;
            break;
            }
        case 0 : {
            date_time = "Воскресенье, " + date_time;
            break;
            }
        default :
            break;
    }
    ofstream userPF(core.user()+".diary", ios_base::app);
    userPF<<date_time<<endl;
    while(true) {
        string buf;
        getline(cin, buf);
        if (buf == "конец_записи")
            break;
        userPF<<buf<<endl;
    }
    userPF.close();
    return;
}*/

//******интерфейс плагина*********

DiaryPlugin::DiaryPlugin(string cmdFile, BaseModule* _parent, SharedObject* _fabric)
    : BaseModule(ModuleInfo("Дневник", "0.1", "21.01.16"), _parent)
{
    testValue = 5;

    ifstream cmdFStr(cmdFile.c_str());
    if (cmdFStr == NULL) {
        cout<<"Отсутствует конфиг файл команд модуля."<<endl;
        exit(1);
    }
    string bufC, bufM;
    auto fnNames = {&DiaryPlugin::test};
    auto it = fnNames.begin();
    auto et = fnNames.end();
    while (it != et) {
        getline(cmdFStr, bufC);
        getline(cmdFStr, bufM);
        add(bufC, *it, bufM);
        ++it;
    }
    cmdFStr.close();

    fabric = _fabric;
}

bool DiaryPlugin::ask(string cmdName, vector<string> cmdArgs)
{
if (cmdArgs.size() == 0 || cmdArgs[0] != "?")
    {
        auto it = connect.find(cmdName);
        if (it != connect.end())
        {
            (this->*(it->second))(cmdArgs);
            return true;
        }
        else
            return false;
    }
else
    {
        auto it = mans.find(cmdName);
        if (it != mans.end())
        {
            cout<<(it->second)<<endl;
            return true;
        }
        else
            return false;
    }
}

extern "C" BaseModule* create(BaseModule* _parent, SharedObject* _fabric)
{
    return new DiaryPlugin("diary.comands.list", _parent, _fabric);
}

extern "C" void destroy(BaseModule* one)
{
    delete one;
    return;
}
