#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <termios.h>
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

#include "core.hpp"
#include "XHR.h"

using namespace std;

mutex commonInQueue_mutex;
mutex commonOutQueue_mutex;
condition_variable input;
atomic<bool> newIn(false);
condition_variable output;
atomic<bool> newOut(false);
atomic<bool> done(false);
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

void readCli(int, string&);
void writeCli(int, string);
void sendTextFile(int, string);

int initServer(int);

void sender(queue<pair<int, string> >&);
void reqHandler(queue<pair<int, string> >&, queue<pair<int, string> >&, map<int, Core*>&);

int main() {
    printHelloMsg();
    map<int, Core*> cores;

    int listen_socket = initServer(8000);

    queue<pair<int, string> > commonInQueue;
    queue<pair<int, string> > commonOutQueue;

    thread senderTh(sender, ref(commonOutQueue));
    thread workerTh(reqHandler, ref(commonInQueue), ref(commonOutQueue), ref(cores));

    while (true) {
        int client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket < 0)
            {cout<<"accept failed"<<endl;break;}
cout<<'x'<<flush;
        string req;
        try
            {readCli(client_socket, req);}
        catch (sock_ex one)
            {cout<<one.what()<<endl;close(client_socket);continue;}
        {
cout<<'y'<<flush;
            unique_lock<mutex> locker(commonInQueue_mutex);
            commonInQueue.push( make_pair(client_socket, req));
cout<<'z'<<flush;
            newIn = true;
            input.notify_one();
cout<<'o'<<flush;
        }
    }
    close(listen_socket);
    done = true;

    workerTh.join();
    senderTh.join();

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

void readCli(int client_socket, string& dest)
{
    const int max_client_buffer_size = 1024;
    char buf[max_client_buffer_size];

    int result = recv(client_socket, buf, max_client_buffer_size, 0);
    if (result < 0) // ошибка получения данных
        throw sock_ex("recv failed");
    else if (result == 0) // соединение закрыто клиентом
        throw sock_ex("connection closed");
    buf[result] = '\0';

    dest = buf;
    return;
}

void writeCli(int client_socket, string source)
{
        // Отправляем ответ клиенту с помощью функции send
//     string temp(source);
    ssize_t result = send(client_socket, source.c_str(),
        source.length(), 0);
    if (result < 0) // произошла ошибка при отправле данных          
        throw sock_ex("send failed");
    return;
}

void sendTextFile(int client_socket, string filePath)
{
    size_t dot = filePath.find_last_of('.');
    string type = filePath.substr(dot+1);
    ifstream file(filePath.c_str(), ifstream::in);
    if (!file.good())
        return;

    stringstream fileBuf;
    string buf;
    while (true)
    {
        getline(file, buf);
        fileBuf<<buf;
        if (!file.eof())
            fileBuf<<endl;
        else
            break;
    }

    XHR temp = confXHR(fileBuf.str().c_str());
    temp.setH("Content-Type", "text/"+type+"; charset=utf-8");
    writeCli(client_socket, temp);
    return;
}

int initServer(int onPort) {
    //КОНФИГУРИРУЕМ И СОЗДАЁМ СОКЕТ
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0)
        {perror("Cannot create a socket");exit(1);}

    //ЗАПОЛНЯЕМ ИНФУ О СЕБЕ
    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_in));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(onPort);        // Port to listen
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int res = bind(listen_socket, (struct sockaddr*) &myaddr, sizeof(myaddr));
    if (res < 0)
        {perror("Cannot bind a socket");exit(1);}

    //ЗАПУСКАЕМ ОЖИДАНИЕ КЛИЕНТА
    struct linger linger_opt = { 1, 0 }; // Linger active, timeout 0
    setsockopt(listen_socket, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
    res = listen(listen_socket, 1);    // "1" is the maximal length of the queue
    if (res < 0)
        {perror("Cannot listen");exit(1);}

    /*   
    //ИНФА КЛИЕНТА
    struct sockaddr_in peeraddr;
    socklen_t peeraddr_len = sizeof(peeraddr);
    int s1 = accept(listen_socket, (struct sockaddr*) &peeraddr, &peeraddr_len);
    if (s1 < 0) 
        {perror("Cannot accept");exit(1);}
    */
    return listen_socket;
}

void sender(queue<pair<int, string> >& toSend)
{
    while (!done)
    {
cout<<'a'<<flush;
        unique_lock<mutex> locker(commonOutQueue_mutex);
        while (!newOut)
            output.wait(locker);

cout<<'b'<<flush;
        while (toSend.size() > 0)
        {
cout<<'c'<<flush;
            writeCli(toSend.front().first, toSend.front().second);
cout<<'d'<<flush;
            close(toSend.front().first);
cout<<'e'<<flush;
            toSend.pop();
        }
cout<<'f'<<flush;
        newOut = false;
    }
    return;
}

//static int requests = 0;

void reqHandler(queue<pair<int, string> >& hin, queue<pair<int, string> >& hout, map<int, Core*>& cores)
{
    while (!done)
    {
cout<<"A"<<flush;
        unique_lock<mutex> lockerI(commonInQueue_mutex);
        while (!newIn)
            input.wait(lockerI);

cout<<"B"<<flush;
        int localClient_socket = hin.front().first;
        XHR localReq(hin.front().second);
        hin.pop();
        if (hin.size() == 0)
            newIn = false;
cout<<"C"<<flush;
        lockerI.unlock();

        string cookie = localReq.getH("Cookie");
        string userIdStr;
        if (cookie.find("user-id=") != string::npos)
            userIdStr = cookie.substr(
                                            cookie.find("user-id=")+8,
                                            cookie.find(";", cookie.find("user-id=")+8) - (cookie.find("user-id=")+8));
        else
            userIdStr = "";
        int userId;
        Core *localCore;
        XHR work;
        if (userIdStr.length() == 0) {
            srand (time(NULL));
            userId = rand() % 0xffffffff;
            {
                unique_lock<mutex> lockerC(cores_mutex);
                localCore = new Core();
                while (cores.find(userId) != cores.end())
                    ++userId;
                cores[userId] = localCore;
            }
            stringstream t; t<<userId; userIdStr = t.str();
            work.setH("Set-Cookie", "user-id="+userIdStr+"; expires=Fri, 1 Jul 2016 03:59:59 GMT; path=/; domain=http://localhost:63342");
            work.setH("withCredentials", "true");
            work.setH("Access-Control-Allow-Credentials", "true");
        }
        else
        {
            userId = atoi(userIdStr.c_str());
            {
                unique_lock<mutex> lockerC(cores_mutex);
                auto cit = cores.find(userId);
                if (cit == cores.end())
                {
                    //прислан ошибочный id, по-хорошему нужно сообщать об ошибке.
                    continue; //В такую погоду свои дома сидят, телевизор смотрют. Только чужие шастают. Не будем дверь открывать!
                }
                else
                    localCore = cit->second;
            }
        }

        stringstream hear;
        streambuf *backup;
        backup = cout.rdbuf();
cerr<<"$$"<<(string)localReq<<"$$\n";
        cout.rdbuf(hear.rdbuf());
        if (localReq.getL()[0] == "POST")
        {
            string cmdName;
            vector<string> cmdArgs;
            string reqStr = localReq.getB();
            parseComand(reqStr, cmdName, cmdArgs);
            try
            { localCore->call(cmdName, cmdArgs); }
            catch (no_fun_ex)
            { }
        }
        else if (localReq.getL()[0] == "GET" && localReq.getL()[1] == "/interface")
        {
            map<string, string> localIFace = localCore->coreIface.getIface();
            auto ifit = localIFace.begin();
            while (ifit != localIFace.end())
            {
                cout << ifit->first << ':' << ifit->second << endl;
                ++ifit;
            }
        }
        cout.rdbuf(backup);

/*++requests;
stringstream t; t<<requests; string re = t.str();
XHR work = confXHR('['+re+"] Got: "+localReq.getB());*/
cout<<'@'<<hear.str()<<'@'<<flush;
        confXHR(work, hear.str());

cout<<"D"<<flush;
        unique_lock<mutex> lockerO(commonOutQueue_mutex);
        hout.push( make_pair(localClient_socket, (string)work));
cout<<"E"<<flush;
        newOut = true;
        lockerO.unlock();
        output.notify_one();
cout<<"F"<<flush;
    }
    return;
}