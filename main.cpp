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

using namespace std;

class bad_req : public std::invalid_argument
{
public:
    bad_req(const std::string& what)
            : std::invalid_argument("Неправильный запрос: " + what + ".") {}
};

void printHelloMsg();
void parseCommand(const json&, string&, vector<string>&);
void reqHandler(FCGX_Request&);
void FcgiToJson(FCGX_Stream*, json&);

map<int, Core*> cores;
mutex cores_mutex;

int main() {
    printHelloMsg();

    int queueLen = 10;
    int socketId;
    FCGX_Init();
    socketId = FCGX_OpenSocket("127.0.0.1:8000", queueLen);
    if (socketId < 0) {
        //ошибка при открытии сокета
        cerr << "Не удаётся открыть сокет" << endl;
        return 1;
    }

    FCGX_Request request;
    if (FCGX_InitRequest(&request, socketId, 0) != 0) {
        //ошибка при инициализации структуры запроса
        cerr << "Не удаётся инициализировать FCGX_Request" << endl;
        return 1;
    }

    thread workerTh(reqHandler, ref(request));

    workerTh.join();
    return 0;
}

void printHelloMsg() {
    cout<<"Это черновой вариант ядра программы для моделирования и изменения объектов."<<endl;
    cout<<"\tВерсия программы от 30 января '16.\n\tРеализована возможность логина (лог действий и дневник) и главное меню."<<endl;
    cout<<"Для вывода списка доступных команд введите <помощь>"<<endl;
    return;
}

void parseCommand(const json& line, string& cmdName, vector<string>& cmdArgs) {
    auto it = line.begin();
    cmdName = *it++;
    auto e = line.end();
    while (it != e)
        cmdArgs.push_back(*it++);
}

void reqHandler(FCGX_Request& request) {
    while (true) {
        int rc = FCGX_Accept_r(&request);
        if (rc < 0) {
            //ошибка при получении запроса
            cerr << "Не удаётся принять запрос" << endl;
            break;
        }
        cerr << "Принят запрос от id=";

        json req, reqMeta, reqData;
        string userIdStr;
        // пока неявно предполагается, что в массиве данных
        // один элемент, а тип информации - text.
        json reqMess;
        try {
            try { FcgiToJson(request.in, req); }
            catch (std::invalid_argument&)
            { throw bad_req("запрос не является json-объектом"); }

            try {
                reqMeta = req.at("meta");
                reqData = req.at("data");
            }
            catch (std::logic_error&)
            { throw bad_req("запрос не соответствует формату "
                                    "{\"meta\": ..., \"data\": ...}"); }

            try { userIdStr = reqMeta.at("user-id"); }
            catch (std::logic_error&)
            { throw bad_req("нет заголовка \"user-id\""); }

            try {
                json firstEntry = reqData.at(0);
                reqMess = firstEntry.at("mess");
            }
            catch (std::logic_error&)
            { throw bad_req("массив данных не соотвествует формату "
                                    "[ {\"type\": ..., \"mess\": ...}, ... ]"); }
        }
        catch (bad_req& br) {
            cerr << br.what() << endl;
            continue;
        }

        int userId = atoi(userIdStr.c_str());
        cerr << userId << "; " << flush;

        Core *localCore;
        if (userId == 0) {
            bad_id: ;
            localCore = new Core();
            srand ((unsigned int)time(NULL));
            userId = rand() % 0xffffffff;
            {
                unique_lock<mutex> lockerC(cores_mutex);
                //если такой userId уже занят, берём следующий
                while (cores.find(userId) != cores.end())
                    ++userId;
                cores[userId] = localCore;
            }
            stringstream t; t<<userId; userIdStr = t.str();
            cerr << "Присвоен id=" << userId << "; " << flush;
        }
        else {  //уже есть user-id
            {
                unique_lock<mutex> lockerC(cores_mutex);
                auto cit = cores.find(userId);
                if (cit == cores.end()) {
                    //прислан ошибочный id
                    cerr << "Ошибочный \"user-id\"!;" << flush;
                    goto bad_id;
                }
                localCore = cit->second;
            }
        }
        json respMeta = { {"user-id", userIdStr} };

        string cmdName;
        vector<string> cmdArgs;
        parseCommand(reqMess, cmdName, cmdArgs);
        cerr << "Вызов: [" << cmdName << "] {";
        for (const string& arg : cmdArgs)
            cerr << arg << ", ";
        cerr << "};" << flush;

        try { localCore->call(cmdName, cmdArgs); }
        catch (no_fun_ex&)
        { cerr << "Обращение к несуществующей функции!;" << flush; }
        catch  (std::exception& e)
        { cerr << e.what() << endl; }
        cerr << endl;

        json respData = localCore->collectOut();
        json resp = { {"meta", respMeta}, {"data", respData} };

        FCGX_PutS("Content-type: text/html\r\n", request.out);
        // Следующий заголовок для cross-domain request,
        // для тестирования клиента с localhost
        FCGX_PutS("Access-Control-Allow-Origin: *\r\n", request.out);
        FCGX_PutS("\r\n", request.out);
        FCGX_PutS(resp.dump().c_str(), request.out);
        FCGX_Finish_r(&request);
    }
}

void FcgiToJson(FCGX_Stream* fin, json& j) {
    stringstream source;
    char line[1024];
    while (FCGX_GetLine(line, 1024, fin) != nullptr)
        source << line;
    source << flush;

    j = json::parse(source);
}
