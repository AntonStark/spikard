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

#include "core.hpp"

using namespace std;

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

void lineToWords(string, string, vector<string>&);

class XHR {
private:
    vector<string> startLine;
    map<string, string> headers;
    bool content;
    string body;
public:
    XHR() {}
    XHR(string source)
    {
        vector<string> parse;
        lineToWords(source, "\r\n", parse);
/*auto y = parse.begin();
while (y != parse.end()) {
    cout<<'*'<<*y<<'*'<<endl;
    ++y;
}*/
        auto it = parse.begin();
        lineToWords(*it, " ", startLine);
        it++;
        while (it != parse.end() && *it != "") 
        {
            vector<string> pair;
            lineToWords(*it, ": ", pair);
            headers[pair[0]] = pair[1];
            it++;
        }
        if (it != parse.end()) 
        {
            it++;
            if (it != parse.end()) 
            {
                body = *it;
                content = true;
            }
            else
                content = false;
        }
// cout<<source<<"\n<>\n"<<this<<endl;
//cout<<"XHR:\n"<<*this<<"конец XHR"<<endl;
    }
    XHR(const XHR& other) :
        startLine(other.startLine),
        headers(other.headers),
        content(other.content),
        body(other.body) {}
    XHR& operator= (XHR other)
    {
        startLine = other.startLine;
        headers = other.headers;
        content = other.content;
        body = other.body;
        return *this;
    }
    operator string() const 
    {
        stringstream buf;
        buf << startLine[0] << ' ' << startLine[1]
            << ' ' << startLine[2] << "\r\n";
        auto it = headers.begin();
        while (it != headers.end()) 
        {
            buf << it->first << ": " << it->second << "\r\n";
            it++;
        }
        buf << "\r\n";
        if (content)
            buf << body;
        buf<<"\0";
        return buf.str();
    }
    
    void setL(string a, string b, string c)
    {
        startLine.resize(3);
        startLine[0] = a; startLine[1] = b; startLine[2] = c;
    }        
    void setH(string key, string value) 
    {
        headers[key] = value;
    }
    void setB(string source) 
    {
cout<<"source="<<source<<endl;
        body = source;
        stringstream ss; ss << body.length(); string len = ss.str();
// string len = "884";
/*int i = 0;
while(len[i++] != string::npos)
cout<<(len[i]=='\0'?'~':len[i]);*/
cout<<"len="<<len<<"<>"<<body.length()<<endl;
        setH("Content-Length", len);
//         headers.insert(make_pair("Content-Length", "884"));
        content = true;
if(content)
cout<<"body="<<endl<<body<<endl;
    }
    vector<string> getL() const 
    {
        return startLine;
    }
    string getB() const 
    {
        return body;
    }
    
};

void printHelloMsg();
void parseComand(string, string&, vector<string>&);
XHR readCli(int);
XHR confXHR(string);
void writeCli(int, XHR);
void sendTextFile(int, string);

int main() {
    printHelloMsg();
    
    
    //КОНФИГУРИРУЕМ И СОЗДАЁМ СОКЕТ
    int listenPort = 8000;
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket < 0) 
        {perror("Cannot create a socket");exit(1);}

    //ЗАПОЛНЯЕМ ИНФУ О СЕБЕ
    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_in));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(listenPort);        // Port to listen
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int res = bind(listen_socket, (struct sockaddr*) &myaddr, sizeof(myaddr));
    if (res < 0) 
        {perror("Cannot bind a socket");exit(1);}

    //ЖДЁМ КЛИЕНТА
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
    

    for (;;) {
cout<<'1'<<endl;
        // Принимаем входящие соединения
        int client_socket = accept(listen_socket, NULL, NULL); //тут новый сокет создаётся
        if (client_socket < 0) 
            {cerr << "accept failed";exit(1);}
/*
stringstream response;
int c = 0;
while (c < 1E6) {
    if (c%1000 == 0)
        response << "\nЭто " << c << " символов.\n";
    else
        response<<"i";
    ++c;
}
response<<endl;
writeCliLine(client_socket, response.str());
*/

        Core core;
      
        XHR req;
cout<<'2'<<endl;
        try
            {req = readCli(client_socket);}
        catch (sock_ex one)
            {cout<<one.what()<<endl;break;}
cout<<"Запрос:\n"<<(string)req<<endl<<"Конец запроса."<<endl;
        vector<string> frstL = req.getL();
//cout<<frstL[0]<<frstL[1]<<frstL[2]<<endl;
        
        string line;
        line = req.getB();
cout<<'3'<<endl;     
cout<<"line="<<line<<endl;
        if (line.size() == 0)
        {
            cout<<"Получена пустая команда = закрытие сессии"<<endl;
            break;
        }

        streambuf *backup;
        stringstream response;

        backup = cout.rdbuf();
        cout.rdbuf(response.rdbuf());
        cout<<core.user()<<">: ";
        
        string cmdName;
        vector<string> cmdArgs;
        parseComand(line, cmdName, cmdArgs);
        try 
            {core.call(core.coreIface[cmdName], cmdArgs);}
        catch (exit_ex)
        {
            cout.rdbuf(backup);
            break;
        }
        catch (no_fun_ex)
            {cout<<"Неизвестная команда."<<endl;}
        cout.rdbuf(backup);
cout<<"Команда обработана."<<endl;
cout<<'4'<<endl;
        
        XHR answer = confXHR(response.str());
        try
            {writeCli(client_socket, answer);}
        catch (sock_ex one)
            {cout<<one.what()<<endl;break;}
        
        // Закрываем соединение с клиентом
        close(client_socket);
        
    }

    close(listen_socket);
    
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

XHR readCli(int client_socket) {
    const int max_client_buffer_size = 1024;
    char buf[max_client_buffer_size];
    int result = recv(client_socket, buf, max_client_buffer_size, 0);
    if (result < 0) // ошибка получения данных
        throw sock_ex("recv failed");
    else if (result == 0) // соединение закрыто клиентом
        throw sock_ex("connection closed");
    buf[result] = '\0';
    
/*cout<<'&';
int i = 0;
    string temp(buf);
while(i != result) {
    if (temp[i] == '\n')
        cout<<'_';
    else if (temp[i] == '\r')
        cout<<'%';
    else
        cout<<temp[i];
    ++i;
}
cout<<'&'<<endl;
*/    
    return XHR(buf);
}

XHR confXHR(string source) {
    XHR temp;
// cout<<source<<endl;
//cout<<"confXHR...";
//cout<<endl<<source<<endl;
//cout<<'1';
//cout.flush();
    temp.setL("HTTP/1.1", "200", "OK");
// cout<<"confXHR1/2="<<endl<<(string)temp<<"/conf"<<endl;
//cout<<'2';
//cout.flush();
    temp.setH("Version", "HTTP/1.1");
    temp.setH("Content-Type", "text/html; charset=utf-8");
    temp.setH("Access-Control-Allow-Origin", /*"http://localhost:63342"*/"http://127.0.0.1");
//cout<<'3'<<endl;
//cout.flush();
// cout<<"confXHR3/4="<<endl<<(string)temp<<"/conf"<<endl;
    temp.setB(source);
// cout<<"confXHR="<<endl<<(string)temp<<"/conf"<<endl;
    return temp;
}

void writeCli(int client_socket, XHR source) {
        // Отправляем ответ клиенту с помощью функции send
//cout<<source<<endl<<sizeof((const char*)source)<<endl;
    string temp(source);
// cout<<temp;
// cout<<"is:"<<temp.length()<<endl;
// cout.flush();
    int result = send(client_socket, temp.c_str(),
        temp.length(), 0);
    if (result < 0) // произошла ошибка при отправле данных          
        throw sock_ex("send failed");
    return;
}

void sendTextFile(int client_socket, string filePath) {
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
//cout<<"sdf"<<endl;
    XHR temp = confXHR(fileBuf.str().c_str());
// cout<<"Отправка:"<<endl<<(string)temp<<endl<<"конец_отправки."<<endl;
// cout.flush();
    temp.setH("Content-Type", "text/"+type+"; charset=utf-8");
    writeCli(client_socket, temp);
    return;
}

void lineToWords(string line, string split, vector<string>& words) {
    size_t i, j;
    i = j = 0;
    while (true) {
        j = line.find(split, i);
        if (j == string::npos) {
            words.push_back(line.substr(i));
            break;
        }
        else {
            words.push_back(line.substr(i,j-i));
        }
        j += split.length();
        i = j;
    }
    return;
}