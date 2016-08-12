#include "./../core.hpp"
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
    int testValue;
    void test(vector<string> cmdArgs) {
        if (cmdArgs.size() != 0)
        {
            if (cmdArgs[0] == "?") 
            {
                cout<<"<тест> - вызов проверочной функции"<<endl;
                return;
            }
            else if (cmdArgs[0] == "*")
            {
                cout<<"тест"<<endl;
                return;
            }
        }
        
        cout<<testValue<<endl;
        for_each(cmdArgs.begin(), cmdArgs.end(), print);
    }
    void diary(vector<string> cmdArgs);

    map<string, void (DiaryPlugin::*)(vector<string>)> methods;
    void methodsCfg();
    SharedObject* fabric;
public:
    DiaryPlugin(BaseModule*, SharedObject*);
    ~DiaryPlugin() {}
    void destroy()
    {
        fabric->destroy(this);
        return;
    }
    
    void ask(string, vector<string>);
    void ifaceCfg();
};

//****функционал плагина****

void DiaryPlugin::diary(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            cout<<"<запись> - Сделать запись в личном дневнике."<<endl;
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"запись"<<endl;
            return;
        }
    }
    
    
    char buffer[60];
    time_t seconds = time(NULL);
    tm *timeinfo = localtime(&seconds);
    char format[] = "%Y, %H:%M:%S";
    strftime(buffer, 60, format, timeinfo);
    string date_time(buffer);

    map<int, string> months, days;
    months[0] = "января";
    months[1] = "февраля";
    months[2] = "марта";
    months[3] = "апреля";
    months[4] = "мая";
    months[5] = "июня";
    months[6] = "июля";
    months[7] = "августа";
    months[8] = "сентября";
    months[9] = "октября";
    months[10] = "ноября";
    months[11] = "декабря";

    days[1] = "Понедельник";
    days[2] = "Вторник";
    days[3] = "Среда";
    days[4] = "Четверг";
    days[5] = "Пятница";
    days[6] = "Суббота";
    days[0] = "Воскресенье";

    date_time = days[timeinfo->tm_wday] +", "+ 
                to_string(timeinfo->tm_mday) +' '+ months[timeinfo->tm_mon] +' '+ 
                date_time;

    ofstream userPF(dynamic_cast<Core*>(getParent())->user()+".diary", ios_base::app);
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
}

//****интерфейс плагина****

DiaryPlugin::DiaryPlugin(BaseModule* _parent, SharedObject* _fabric)
    : BaseModule(ModuleInfo("Дневник", "0.1", "21.01.16"), _parent)
{
    testValue = 5;

    methodsCfg();
    fabric = _fabric;
}

void DiaryPlugin::methodsCfg()
{     
    methods.insert(make_pair("test", &DiaryPlugin::test));
    methods.insert(make_pair("diary", &DiaryPlugin::diary));
    return;
}

void DiaryPlugin::ask(string cmdName, vector<string> cmdArgs)
{
    (this->*methods[cmdName])(cmdArgs);
    return;
}

void DiaryPlugin::ifaceCfg()
{
    BaseModule* mod = this;
    while (mod->getParent())
        mod = mod->getParent();
    Core* root = static_cast<Core*>(mod);
    
    string pseudo;
    stringstream hear;
    streambuf *backup;
    
    backup = cout.rdbuf();
    cout.rdbuf(hear.rdbuf());
    auto mt = methods.begin();
    while (mt != methods.end()) {
        (this->*(mt->second))(vector<string>({"*"}));   //перехватываем ключевое слово для функции
        getline(hear, pseudo);
        root->coreIface.add(pseudo, mt->first, this);
        mt++;
    }
    cout.rdbuf(backup);
    return;
}

extern "C" BaseModule* create(BaseModule* _parent, SharedObject* _fabric)
{
    return new DiaryPlugin(_parent, _fabric);
}

extern "C" void destroy(BaseModule* one)
{
    delete one;
    return;
}