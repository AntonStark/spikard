using namespace std;

ModuleInfo coreInfo("Феникс", "Хи", "21.01.16");

class no_cfg_file {};
class exit_ex {};
class no_fun_ex {};
class sh_obj_err : public std::exception
{
private:
    std::string mess;
public:
    sh_obj_err(const char *msg)
        : exception(), mess(msg) {}
    virtual const char* what() const noexcept
    {
        return mess.c_str();
    }
    virtual ~sh_obj_err() {}
};

bool BaseModule::find(string cmdName, vector<string> cmdArgs)
{//обычный обход дерева подключенныx от потомков к родителям, от последних к первым
    for (list<BaseModule*>::reverse_iterator rit = modules.rbegin(); rit != modules.rend(); rit++)
    {
        if (*rit != this)
        {
            if ( (*rit)->find(cmdName, cmdArgs)) return true;
        }
        else
        {
            if ( (*rit)->ask(cmdName, cmdArgs)) return true;
        }
    }
    return false;
}

SharedObject::SharedObject(const std::string& fullPath, int flags)
    : libHandle(NULL)
{
    libHandle = dlopen(fullPath.c_str(), flags);
    if (libHandle == NULL)
    {
        char* err = ::dlerror();
        if (err)
            throw sh_obj_err(err);
        else
            throw sh_obj_err("");
    }
    ::dlerror();
    *(void**)(&createPlugin) = dlsym(libHandle, "create");
    const char* dlsym_error = ::dlerror();
    if (createPlugin == NULL)
        if (dlsym_error)
            throw sh_obj_err(dlsym_error);
        else
            throw sh_obj_err("");
    ::dlerror();
    *(void**)(&destroyPlugin) = dlsym(libHandle, "destroy");
    dlsym_error = ::dlerror();
    if (destroyPlugin == NULL)
        if (dlsym_error)
            throw sh_obj_err(dlsym_error);
        else
            throw sh_obj_err("");
}

SharedObject::~SharedObject()
{
    while (!items.empty())
        destroy(*items.begin());
    if (libHandle != NULL)
        dlclose(libHandle);
}

BaseModule* SharedObject::create(BaseModule* _parent)
{   //закидываем указатель на созданный элемент в items return *(pair.first)
    BaseModule* plug = (*createPlugin)(_parent, this);
    return *(items.insert(plug).first);
}

void SharedObject::destroy(BaseModule* that)
{
    std::set<BaseModule*>::iterator it = items.find(that);
    if (it != items.end())
    {
        that->getParent()->deregisterModule(that);
        (*destroyPlugin)(*it);
        items.erase(it);
    }
    else
        throw sh_obj_err("Попытка уничтожения чужим деструктором.");
    return;
}

void Core::noreload_init()
{
    noreload.insert(make_pair("вход",this));
    //...
    return;
}

Core::Core(string cmdFile) :
    BaseModule(coreInfo)
{
    userName = "?";

    ifstream cmdFStr(cmdFile.c_str());
    if (cmdFStr == NULL) {
        throw no_cfg_file(/*this.getModuleInfo().name*/);
    }
    string bufC, bufM;
    auto fnNames = {&Core::emptyComand, &Core::logIn, &Core::logOut, &Core::diary,
                    &Core::end, &Core::plugIn, &Core::getMan, &Core::change};
    auto it = fnNames.begin();
    auto et = fnNames.end();
    while (it != et) {
        getline(cmdFStr, bufC);
        getline(cmdFStr, bufM);
        add(bufC, *it, bufM);
        ++it;
    }
    cmdFStr.close();

    noreload_init();
}

bool Core::ask(string cmdName, vector<string> cmdArgs)
{
if (cmdArgs.size() == 0 || cmdArgs[0] != "?")
    {
        auto it = connect.find(cmdName);
        if (it != connect.end())
        {
            (this->*(it->second))(cmdArgs);
            return true;
        }
        else
            return false;
    }
else
    {
        auto it = mans.find(cmdName);
        if (it != mans.end())
        {
            cout<<(it->second)<<endl;
            return true;
        }
        else
            return false;
    }
}

void Core::call(string cmdName, vector<string> cmdArgs)
{
    map<string, BaseModule*>::iterator it = noreload.find(cmdName); //сначала проверяем словарь неперегружаемых команд
    if (it != noreload.end())
    {
        if (it->second->ask(cmdName, cmdArgs)) //прицельный запрос
            return;
        else
            throw no_fun_ex();
    }
    else
    {
        if (find(cmdName, cmdArgs))  //поиск по древу модулей
            return;
        else
            throw no_fun_ex();
    }
}

/*****функционал_ядра*****************/
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
        
        static struct termios stored_settings;
        struct termios new_settings;
        tcgetattr(0,&stored_settings);
        new_settings = stored_settings;
        new_settings.c_lflag &= (~ECHO);
        tcsetattr(0,TCSANOW,&new_settings);
        getline(cin, pass);
        tcsetattr(0,TCSANOW,&stored_settings);
        
        string hash(32, (char)0);
        for (int i = 0; i < 32; ++i) {
            for (int j = 0; j < pass.length(); ++j)
                hash[i] += (char)((int)pass[j]*3^(j+i));
            hash[i] = (char)((unsigned int)hash[i]%95 + 32);
        }
        pass = "0";
        
        if (hash != password)
            cout<<"Неверный пароль"<<endl;
        else
            userName = name;
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

        static struct termios stored_settings;
        struct termios new_settings;
        tcgetattr(0,&stored_settings);
        new_settings = stored_settings;
        new_settings.c_lflag &= (~ECHO);
        tcsetattr(0,TCSANOW,&new_settings);
        getline(cin, pass);
        tcsetattr(0,TCSANOW,&stored_settings);
        
        string hash(32, (char)0);
        for (int i = 0; i < 32; ++i) {
            for (int j = 0; j < pass.length(); ++j)
                hash[i] += (char)((int)pass[j]*3^(j+i));
            hash[i] = (char)((unsigned int)hash[i]%95 + 32);
        }
        pass = "0";

        string put = name + " " + hash;
        cout<<"Добавление нового пользователя..."<<endl;
        ofstream usersFile("users.list", ios_base::app);
        usersFile<<put<<endl;
        usersFile.close();
    }
    return;
}

void Core::logOut(vector<string> cmdArgs) {
    userName = "?";
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
    throw exit_ex();
    return;
}

void Core::plugIn(vector<string> cmdArgs) {
    if (cmdArgs.size() == 0 || cmdArgs[0].length() == 0)
    {
        cout<<"Пропущено имя плагина."<<endl;
        return;
    }
    string pluginFullName = "./plugins/"+cmdArgs[0]+".so";
    map<string, SharedObject*>::iterator it = SO_inWork.find(pluginFullName);
    try
    {
        if (it == SO_inWork.end())
            SO_inWork.insert(
                make_pair(pluginFullName, new SharedObject(pluginFullName, RTLD_LAZY))
                );
        registerModule(SO_inWork[pluginFullName]->create(this));
    }
    catch (sh_obj_err ex)
    {
        cout<<"Возникла ошибка при попытке подключения плагина "<<pluginFullName<<':'<<endl;
        cout<<ex.what()<<endl;
    }
    return;
}

void Core::printListOfComands(vector<string> cmdArgs) {
    cout<<"Список доступных команд:"<<endl;
    cout<<"<вход>\n\t- Представиться системе."<<endl;
    cout<<"\tТакже возможен ограниченный анонимный доступ."<<endl;
    cout<<"<выход>\n\t- Вернуться к анонимности."<<endl;
    cout<<"<запись>\n\t- Сделать запись в личном дневнике."<<endl;
    cout<<"<конец>\n\t- Завершение работы программы."<<endl;
    cout<<"<отключить [список имён]>\n\t- Выигрузить модули из списка."<<endl;
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
        call(cmdArgs[0],vector<string>({"?"}));
    return;
}
