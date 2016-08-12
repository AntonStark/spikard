#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <termios.h>
#include <dlfcn.h>
#include <vector>
#include <map>

using namespace std;

void printHelloMsg();
void printListOfComands();
void logIn();
void logOut();
void plugIn();//    НЕ
void change();//  РЕАЛИЗОВАНЫ
void diary();
void echo_off();
void echo_on();
string myHash(string);
  void dlTest();

string userName = "?";
static struct termios stored_settings;

int main() {
    printHelloMsg();
    while (true) {
        string line;
        cout<<userName<<">: ";
        getline(cin, line);
        string cmdName = line.substr(0, line.find(' '));

        if (cmdName == "вход")
            logIn();
        else if (cmdName == "выход")
            logOut();
else if (cmdName == "тест") {
    dlTest();
}
        else if (cmdName == "подключить")
            plugIn();
        else if (cmdName == "править")
            change();
        else if (cmdName == "помощь")
            printListOfComands();
        else if (cmdName == "")
            continue;
       else if (cmdName == "конец")
            break;
        else if (cmdName == "запись")
            if (userName != "?")
                diary();
            else
                cout<<"Недоступно."<<endl;
        else
            cout<<"Команда не распознана."<<endl;
    }
    return 0;
}

void printHelloMsg() {
    cout<<"Это черновой вариант ядра программы для моделирования и изменения объектов."<<endl;
    cout<<"\tВерсия программы от 6 января '16.\n\tРеализована возможность логина (лог действий и дневник) и главное меню."<<endl;
    cout<<"Для вывода списка доступных команд введите <помощь>"<<endl;
    return;
}

void printListOfComands() {
    cout<<"Список доступных команд:"<<endl;
    cout<<"<вход>\n\t- Представиться системе."<<endl;
    cout<<"\tТакже возможен ограниченный анонимный доступ."<<endl;
    cout<<"<выход>\n\t- Вернуться к анонимности."<<endl;
    cout<<"<запись>\n\t- Сделать запись в личном дневнике."<<endl;
    cout<<"<конец>\n\t- Завершение работы программы."<<endl;
    cout<<"<подключить [список имён]>\n\t- Чтобы загрузить модули из списка,\n\tдоступно автодополнение."<<endl;
    cout<<"<помощь>\n\t- Вывод этого сообщения."<<endl;
    cout<<"<править [имя модуля]>\n\t- Для изменения модуля (в том числе подключенного)."<<endl;
    return;
}

void logIn() {
    if (ifstream("users.list") == NULL)
        ofstream("users.list");

    cout<<"Введите имя пользователя.\n\tПустая строка для регистрации нового участника."<<endl;
    string name, pass;
    getline(cin, name);
    int i;
    while((i=name.find(' ')) != string::npos)
        name.erase(i,1);

    if (name.length() != 0) {
        ifstream usersFile("users.list");
        int found = 0;
        string buf, login, password;
        while (!usersFile.eof()) {
            getline(usersFile, buf);
            login = buf.substr(0, buf.find(' '));
            if (login == name) {
                found = 1;
                password = buf.substr(buf.find(' ')+1);
                break;
            }
        }
        usersFile.close();
        if (found == 0) {
            cout<<"Пользователь с таким именем не найден."<<endl;
            return;
        }
        cout<<"Введите пароль."<<endl;
        echo_off();
        getline(cin, pass);
        echo_on();
        if (myHash(pass) != password)
            cout<<"Неверный пароль"<<endl;
        else
            userName = name;
        pass = "0";
    }

    else {
        cout<<"Создание новой учётной записи."<<endl;
        cout<<"Введите желаемое имя пользователя.\n\tПробелы игнорируются.\n\tПустая строка - отмена."<<endl;
        while (true) {
            getline(cin, name);
            if (name.length() == 0)
                return;
            while((i=name.find(' ')) != string::npos)
                name.erase(i,1);
            int match = 0;
            ifstream usersFile("users.list");
            string buf, login;
            while (!usersFile.eof()) {
                getline(usersFile, buf);
                login = buf.substr(0, buf.find(' '));
                if (login == name) {
                    match = 1;
                    break;
                }
            }
            usersFile.close();
            if (match == 1) {
                cout<<"Это имя занято. Попробуйте другое."<<endl;
                continue;
            }
            else 
                break;
        }
        cout<<"Задайте пароль."<<endl;
        echo_off();
        getline(cin,pass);
        echo_on();
        string put = name + " " + myHash(pass);
        pass = "0";
        cout<<"Добавление нового пользователя..."<<endl;
        ofstream usersFile("users.list", ios_base::app);
        usersFile<<put<<endl;
        usersFile.close();
    }

    return;
}

void logOut() {
    userName = "?";
    return;
}

void diary() {
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
    ofstream userPF(userName+".diary", ios_base::app);
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

void plugIn() {
    return;
}

void change() {
    return;
}

void echo_off(void) {
    struct termios new_settings;
    tcgetattr(0,&stored_settings);
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ECHO);
    tcsetattr(0,TCSANOW,&new_settings);
    return;
}

void echo_on(void) {
    tcsetattr(0,TCSANOW,&stored_settings);
    return;
}

string myHash(string source) {
    string hash(32, (char)0);
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < source.length(); ++j)
            hash[i] += (char)((int)source[j]*3^(j+i));
        hash[i] = (char)((unsigned int)hash[i]%95 + 32);
    }
    return hash;
}

void dlTest() {
    int arg;
    cin>>arg;
    void* testplugin_handler;
    testplugin_handler = dlopen("./plugins/testplugin.so", RTLD_LAZY);
    if (!testplugin_handler) {
        cout<<"dlopen() error: "<<dlerror()<<endl;
        exit(1);
    }
    void (*test)(int);
    *(void**)(&test) = dlsym(testplugin_handler, "test");
    if (test == NULL) {
        cout<<"Function not found."<<endl;
        exit(1);
    }
    (*test)(arg);
    dlclose(testplugin_handler);
    return;
}
