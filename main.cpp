#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <dlfcn.h>
#include <vector>
#include <map>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "fcgiapp.h"

#include "core.hpp"
#include "InnerRequest.hpp"

using namespace std;

mutex cores_mutex;

class sock_ex : public exception
{
private:
    std::string mess;
public:
    sock_ex(const char *msg)
        : exception(), mess(msg) {}
    virtual const char* what() const noexcept
    {
        return mess.c_str();
    }
    virtual ~sock_ex() {}
};

void printHelloMsg();
void parseComand(string, string&, vector<string>&);

void reqHandler(map<int, Core*>&);

void FcgiToInnerReq(FCGX_Stream*, InnerRequest&);

int socketId;

int main() {
    printHelloMsg();
    map<int, Core*> cores;

    int queueLen = 10;
    FCGX_Init();
    socketId = FCGX_OpenSocket("127.0.0.1:8000", queueLen);
    if (socketId < 0)
    {
        //ошибка при открытии сокета
        return 1;
    }

    thread workerTh(reqHandler, ref(cores));

    workerTh.join();

    return 0;
}

void printHelloMsg()
{
    cout<<"Это черновой вариант ядра программы для моделирования и изменения объектов."<<endl;
    cout<<"\tВерсия программы от 30 января '16.\n\tРеализована возможность логина (лог действий и дневник) и главное меню."<<endl;
    cout<<"Для вывода списка доступных команд введите <помощь>"<<endl;
    return;
}

void parseComand(string line, string& cmdName, vector<string>& cmdArgs)
{
    unsigned long s = line.find(' ');
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

void reqHandler(map<int, Core*>& cores)
{
    int rc;
    FCGX_Request request;

    if (FCGX_InitRequest(&request, socketId, 0) != 0)
    {
        //ошибка при инициализации структуры запроса
        cerr << "Не удаётся инициализировать FCGX_Request" << endl;
        return;
    }
    cout << "FCGX_Request инициализирован" << endl;

    while (true)
    {
        rc = FCGX_Accept_r(&request);
        if (rc < 0)
        {
            //ошибка при получении запроса
            cerr << "Не удаётся принять запрос" << endl;
            break;
        }
        cout << "Запрос принят" << endl;

        InnerRequest cliRequest, cliRespond;
        FcgiToInnerReq(request.in, cliRequest);

        Core *localCore;
        string userIdStr = cliRequest.getH("user-id");
        if (userIdStr.size() == 0)
        {
            cerr << "В запрсое нет заголовка \"user-id\"" << endl;
            continue;
        }
        int userId = atoi(userIdStr.c_str());
        if (userId == 0)
        {
            srand ((unsigned int)time(NULL));
            userId = rand() % 0xffffffff;
            {
                unique_lock<mutex> lockerC(cores_mutex);
                localCore = new Core();
                //если такой userId уже занят, берём следующий
                while (cores.find(userId) != cores.end())
                    ++userId;
                cores[userId] = localCore;
            }
            stringstream t; t<<userId; userIdStr = t.str();
            cliRespond.setH("user-id", userIdStr);
        }
        else    //уже есть user-id
        {
            {
                unique_lock<mutex> lockerC(cores_mutex);
                auto cit = cores.find(userId);
                if (cit == cores.end())
                {
                    //прислан ошибочный id
                    cerr << "Ошибочный \"user-id\"" << endl;
                    continue;
                }
                localCore = cit->second;
                cliRespond.setH("user-id",userIdStr);
            }
        }

        stringstream hear;
        streambuf *backup;
        backup = cout.rdbuf();
        cout.rdbuf(hear.rdbuf());
        string reqStr = cliRequest.getB();
        string cmdName;
        vector<string> cmdArgs;
        parseComand(reqStr, cmdName, cmdArgs);
        try
        { localCore->call(cmdName, cmdArgs); }
        catch (no_fun_ex)
        { cerr << "Обращение к несуществующей функции" << endl; }
        cout.rdbuf(backup);
        cliRespond.setB(hear.str());

        FCGX_PutS("Content-type: text/html\r\n", request.out);
        FCGX_PutS("\r\n", request.out);
        FCGX_PutS(cliRespond.toStr().c_str(), request.out);
        FCGX_Finish_r(&request);
    }
    return;
}

void FcgiToInnerReq(FCGX_Stream* fin, InnerRequest& target)
{
    stringstream source;
    char line[1024];
    while (FCGX_GetLine(line, 1024, fin) != nullptr)
        source << line;
    source << flush;
//std::cerr<<"Check-in:\n"<<source.str()<<std::endl;

    target.configure(source.str());
    return;
}