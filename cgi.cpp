#include <string>
#include "fcgi_stdio.h"
#include <stdlib.h>
#pragma comment(lib, "libfcgi.lib")

int main(int argc, char* const argv[] )
{
    std::string port=":9000";        //Задаем номер порта TCP
    int  listenQueueBacklog = 400;    //Глубина стека запросов
    FCGX_Stream *in, *out, *err;
    FCGX_ParamArray envp;


    if(FCGX_Init())    exit(1); //Инициализируем библиотеку перед работой.

    int  listen_socket = FCGX_OpenSocket(port.c_str(), listenQueueBacklog); //Открываем новый слушающий сокет
    if(listen_socket < 0)    exit(1);

    FCGX_Request request;
    if(FCGX_InitRequest(&request,  listen_socket, 0)) exit(1); //Инициализируем структуру запроса

    while(FCGX_Accept_r(&request) == 0)
    {
        FCGX_FPrintF(request.out, "Content-type: text/html\r\n\r\n<TITLE>fastcgi</TITLE>\n<H1>Fastcgi: Hello world.</H1>\n");

        FCGX_Finish_r(&request);//Завершаем запрос
    }

    return 0;
}