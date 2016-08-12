#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <initializer_list>

using namespace std;

extern void echo_off();
extern void echo_on();
extern string myHash(string);

class exit_ex {};

/*void emptyComand(vector<string>);
void logIn(vector<string>);
void logOut(vector<string>);
void diary(vector<string>);
void end(vector<string>);
void plugIn(vector<string>);
void getMan(vector<string>);
void change(vector<string>);*/

Core::Core(string cmdFile) {
    ifstream cmdFStr(cmdFile.c_str());
    if (cmdFStr == NULL) {
        cout<<"Отсутствует конфиг-файл команд."<<endl;
        exit(1);//Вообще-то плохо
    }
    string bufC, bufM;
    auto fnNames = {&Core::emptyComand, &Core::logIn, &Core::logOut, &Core::diary,
                    &Core::end, &Core::plugIn, &Core::getMan, &Core::change};
    auto it = fnNames.begin();
    auto et = fnNames.end();
    while (it != et) {
        getline(cmdFStr, bufC);
        getline(cmdFStr, bufM);
        interface.add(bufC, *it, bufM);
        ++it;
    }
    cmdFStr.close();

    modules.push_front(this);

    run = true;
    userName = "?";
}

Core::~Core() {
    while (modules.size() > 1)
        delete modules.back();
}

void Core::call(string cmdName, vector<string> cmdArgs) {
    auto it = interface.connect.find(cmdName);
    if (it == interface.connect.end())
        cout<<"Неизвестная команда."<<endl;
    else 
        (this->*(it->second))(cmdArgs);
    return;
}

void Core::emptyComand(vector<string> cmdArgs) {
    return;
}

void Core::logIn(vector<string> cmdArgs) {
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

void Core::logOut(vector<string> cmdArgs) {
    this->userName = "?";
    return;
}

void Core::diary(vector<string> cmdArgs) {
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

void Core::end(vector<string> cmdArgs) {
    //run = false;
    throw exit_ex();
    return;
}

void Core::plugIn(vector<string> cmdArgs) {
    //СДЕЛАТЬ ПОДКЛЮЧЕНИЕ ПЛАГИНОВ
    return;
}

void Core::printListOfComands(vector<string> cmdArgs) {
    cout<<"Список доступных команд:"<<endl;
    cout<<"<вход>\n\t- Представиться системе."<<endl;
    cout<<"\tТакже возможен ограниченный анонимный доступ."<<endl;
    cout<<"<выход>\n\t- Вернуться к анонимности."<<endl;
    cout<<"<запись>\n\t- Сделать запись в личном дневнике."<<endl;
    cout<<"<конец>\n\t- Завершение работы программы."<<endl;
    cout<<"<подключить [список имён]>\n\t- Чтобы загрузить модули из списка,\n\tдоступно автодополнение."<<endl;
    cout<<"<помощь [имя команды]>\n\t- Вывод справки о команде."<<endl;
    cout<<"<править [имя модуля]>\n\t- Для изменения модуля (в том числе подключенного)."<<endl;
    return;
}

void Core::change(vector<string> cmdArgs) {
    return;
}

void Core::getMan(vector<string> cmdArgs) {
    if (cmdArgs.size() == 0)
        printListOfComands(cmdArgs);
    else
        cout<<interface.mans[cmdArgs[0]]<<endl;
    return;
}
