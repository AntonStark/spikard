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

void collapseBr(string& line, list<string>& collapsed)
{
    size_t from, b, bb;
    from = 0;
    while (true)
    {
        b = line.find('"', from);
        if (b == string::npos)
            return;
        bb = line.find('"', b + 1);
        if (bb == string::npos)
            return;
        collapsed.push_back(line.substr(b + 1, bb - b - 1));
        line.replace(b, bb - b + 1, " \"\" "); //Длина содержимого bb-b-1. +2 сами кавычки = bb-b+1
        from = b+3; //На b теперь находится ' ' и дальше два '\"'
    }
}

void expandBr(vector<string>& cmdArgs, list<string> collapsed)
{
    for (int i = 0; i < cmdArgs.size(); ++i)
    {
        if (collapsed.empty())
            break;
        if (cmdArgs[i] == "\"\"")
        {
            cmdArgs[i] = collapsed.front();
            collapsed.pop_front();
        }
    }
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

        list<string> collapsed;
        collapseBr(line, collapsed);

        s = line.find(' ');
        while (s != string::npos) {
            if (s != 0)
                cmdArgs.push_back(line.substr(0, s));
            line.erase(0, s+1);
            s = line.find(' ');
        }
        if (!line.empty())
            cmdArgs.push_back(line);

        expandBr(cmdArgs, collapsed);
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
        cerr << "Принят запрос от id=";

        InnerRequest cliRequest, cliRespond;
        try
        { FcgiToInnerReq(request.in, cliRequest); }
        catch (bad_req_ex ex)
        { cerr << "Неправильный запрос: " << ex.what() << "!;" << endl; continue; }

        Core *localCore;
        string userIdStr = cliRequest.getH("user-id");
        int userId = atoi(userIdStr.c_str());
        cerr << userId << "; " << flush;

        if (userId == 0)
        {
            bad_id: ;
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
            cerr << "Присвоен id=" << userId << "; " << flush;
        }
        else    //уже есть user-id
        {
            {
                unique_lock<mutex> lockerC(cores_mutex);
                auto cit = cores.find(userId);
                if (cit == cores.end())
                {
                    //прислан ошибочный id
                    cerr << "Ошибочный \"user-id\"!;" << flush;
                    goto bad_id;
                }
                localCore = cit->second;
                cliRespond.setH("user-id",userIdStr);
            }
        }

        string reqStr = cliRequest.getB();
        stringstream hear;
        streambuf *backup;
        backup = cout.rdbuf();
        cout.rdbuf(hear.rdbuf());

        string cmdName;
        vector<string> cmdArgs;
        parseComand(reqStr, cmdName, cmdArgs);
        cerr << "Вызов: [" << cmdName << "] {";
        for (string arg : cmdArgs)
            cerr << arg << ", ";
        cerr << "};" << flush;

        try
        { localCore->call(cmdName, cmdArgs); }
        catch (no_fun_ex)
        { cerr << "Обращение к несуществующей функции!;" << flush; }
        catch (add_handler& hInfo)
        { cliRespond.setH(hInfo.key, hInfo.value); }
        catch  (std::exception& e)
        { cerr << e.what() << endl; }

        cout.rdbuf(backup);
        cliRespond.setB(hear.str());

        FCGX_PutS("Content-type: text/html\r\n", request.out);
        // Следующий заголовок для cross-domain request,
        // для тестирования клиента с localhost
        FCGX_PutS("Access-Control-Allow-Origin: *\r\n", request.out);
        FCGX_PutS("\r\n", request.out);
        FCGX_PutS(cliRespond.toStr().c_str(), request.out);
        FCGX_Finish_r(&request);
        cout << endl << flush;
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

    target.configure(source.str());
    return;
}