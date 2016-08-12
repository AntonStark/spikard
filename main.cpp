#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <termios.h>
#include <dlfcn.h>
#include <vector>
#include <map>
//#include <stdlib.h>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "core.hpp"

using namespace std;

void printHelloMsg();
void parseComand(string, string&, vector<string>&);

int main() {
    printHelloMsg();
    Core core;
    
int listenPort = 8000;
int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
if (listen_socket < 0) {
    perror("Cannot create a socket");
    core.~Core();
    exit(1);
}
struct sockaddr_in myaddr;
memset(&myaddr, 0, sizeof(struct sockaddr_in));
myaddr.sin_family = AF_INET;
myaddr.sin_port = htons(listenPort);        // Port to listen
myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
int res = bind(listen_socket, (struct sockaddr*) &myaddr, sizeof(myaddr));
if (res < 0) {
    perror("Cannot bind a socket");
    core.~Core();
    exit(1);
}
struct linger linger_opt = { 1, 0 }; // Linger active, timeout 0
setsockopt(listen_socket, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
res = listen(listen_socket, 1);    // "1" is the maximal length of the queue
if (res < 0) {
    perror("Cannot listen");
    core.~Core();
    exit(1);
}
struct sockaddr_in peeraddr;
socklen_t peeraddr_len = sizeof(peeraddr);
int s1 = accept(listen_socket, (struct sockaddr*) &peeraddr, &peeraddr_len);
if (s1 < 0) {
    perror("Cannot accept");
    core.~Core();
    exit(1);
}

const int max_client_buffer_size = 1024;
char buf[max_client_buffer_size];

for (;;) {
    // Принимаем входящие соединения
    int client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket < 0) {
        cerr << "accept failed";
        core.~Core();
        exit(1);
    }

    int result = recv(client_socket, buf, max_client_buffer_size, 0);

    stringstream response; // сюда будет записываться ответ клиенту
    stringstream response_body; // тело ответа

    if (result < 0) {
        // ошибка получения данных
        cerr << "recv failed: " << result << "\n";
        close(client_socket);
    } else if (result == 0) {
        // соединение закрыто клиентом
        cerr << "connection closed...\n";
    } else if (result > 0) {
        // Мы знаем фактический размер полученных данных, поэтому ставим метку конца строки
        // В буфере запроса.
        buf[result] = '\0';
cout<<"sdfasdf"<<endl;
        // Данные успешно получены
        // формируем тело ответа (HTML)
        response_body << "<title>Test C++ HTTP Server</title>\n"
            << "<h1>Test page</h1>\n"
            << "<p>This is body of the test page...</p>\n"
            << "<h2>Request headers</h2>\n"
            << "<pre>" << buf << "</pre>\n"
            << "<em><small>Test C++ Http Server</small></em>\n";

        // Формируем весь ответ вместе с заголовками
        response << "HTTP/1.1 200 OK\r\n"
            << "Version: HTTP/1.1\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            <<"Access-Control-Allow-Origin: http://localhost:63342\r\n"
            << "Content-Length: " << response_body.str().length()
            << "\r\n\r\n"
            << response_body.str();

        // Отправляем ответ клиенту с помощью функции send
        result = send(client_socket, response.str().c_str(),
            response.str().length(), 0);

        if (result < 0) {
            // произошла ошибка при отправле данных
            cerr << "send failed";
        }
        // Закрываем соединение к клиентом
        close(client_socket);
    }
}


    
    while (true) {
/*auto o = core.IFace.begin();
while (o != core.IFace.end()) {
    cout<<o->first<<'~'<<o->second<<endl;
    o++;
}*/
        cout<<core.user()<<">: ";
        string line, cmdName;
        vector<string> cmdArgs;
        getline(cin, line);
        if (line.size() == 0)
            continue;
        parseComand(line, cmdName, cmdArgs);
        try 
        {
            core.call(core.coreIface[cmdName], cmdArgs);
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
    return 0;
}

void printHelloMsg() {
    cout<<"Это черновой вариант ядра программы для моделирования и изменения объектов."<<endl;
    cout<<"\tВерсия программы от 30 января '16.\n\tРеализована возможность логина (лог действий и дневник) и главное меню."<<endl;
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
