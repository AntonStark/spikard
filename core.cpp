#include "core.hpp"

using namespace std;

ModuleInfo coreInfo("Феникс", "Фи", "30.01.16");

std::string toStr(const INFO_TYPE& infoType)
{
    switch (infoType)
    {
        case INFO_TYPE::ANCL :
            return "ancillary";
        case INFO_TYPE::TXT :
            return "text";
        case INFO_TYPE::TEX_EXP :
            return "tex_expression";
        case INFO_TYPE::ML_OBJ :
            return "mathlang_object";
        case INFO_TYPE::ERR :
            return "error";
        case INFO_TYPE::NAME :
            return "mathlang_name";
    }
}

void BaseModule::ifaceRefresh()
{
    if (getParent() == NULL) //запущена для ядра
        static_cast<Core*>(this)->coreIface.clear();
    //обычный обход дерева подключенныx, от потомков к родителям, от последних к первым
    for (list<BaseModule*>::reverse_iterator rit = modules.rbegin(); rit != modules.rend(); rit++)
        if (*rit != this)
            (*rit)->ifaceRefresh();
        else
            (*rit)->ifaceCfg();
    return;
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
{//закидываем указатель на созданный элемент в items return *(pair.first)
    BaseModule* plug = (*createPlugin)(_parent, this);
    return *(items.insert(plug).first);
}

void SharedObject::destroy(BaseModule* that)
{
    set<BaseModule*>::iterator it = items.find(that);
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

void IFace::add(std::string keyWord, std::string funName, BaseModule* contain)
{   
    //пока подход упрощённый. если это funName уже встречается (пусть в другом модуле),
    //то запись не добавляется. если уже используется keyWord, то keyWord=keyWord+funName         
    if (index.find(funName) != index.end() )
        return;
    if (iface.find(keyWord) != iface.end() )
        keyWord = keyWord + "_in_" + contain->getModuleInfo().name;
    index.insert(make_pair(funName, contain));
    iface.insert(make_pair(keyWord, funName));
}

void IFace::clear()
{
    index.clear();
    iface.clear();
}

map<string, string> IFace::getIface() const
{
    return iface;
}

BaseModule* IFace::where(std::string funName)
{
    auto it = index.find(funName);
    if (it == index.end())
        throw no_fun_ex();
    else
        return it->second;
}

std::string IFace::operator[] (std::string keyWord)
{
    auto kw = iface.find(keyWord);
    if (kw == iface.end() )
        throw no_fun_ex();
    else 
        return kw->second;
}

/**ядро**/
void Core::methodsCfg()
{
    methods.insert(make_pair("logIn", &Core::logIn));
    methods.insert(make_pair("logOut", &Core::logOut));
    methods.insert(make_pair("end", &Core::end));
    methods.insert(make_pair("plugIn", &Core::plugIn));
    methods.insert(make_pair("plugOut", &Core::plugOut));
    methods.insert(make_pair("getInterface", &Core::getInterface));
    methods.insert(make_pair("getMan", &Core::getMan));
}

void Core::noreload_init()
{
    noreload.insert(make_pair("logIn",this));
    //...
    return;
}

Core::Core() : 
    BaseModule(coreInfo)
{
    userName = "?";

    methodsCfg();
    noreload_init();
    ifaceRefresh();
}

void Core::ifaceCfg()
{
    BaseModule* mod = this;
    while (mod->getParent())
        mod = mod->getParent();
    Core* root = static_cast<Core*>(mod);
    
    string pseudo;
    stringstream hear;
    streambuf *backup;
    
    backup = cout.rdbuf();
    cout.rdbuf(hear.rdbuf());
    auto mt = methods.begin();
    while (mt != methods.end()) {
        (this->*(mt->second))(vector<string>({"*"}));   //перехватываем ключевое слово для функции
        getline(hear, pseudo);
        root->coreIface.add(pseudo, mt->first, this);
        mt++;
    }
    cout.rdbuf(backup);
    return;
}

void Core::ask(string cmdName, vector<string> cmdArgs)
{
    (this->*methods[cmdName])(cmdArgs);
}

void Core::call(string cmdName, vector<string> cmdArgs)
{
    map<string, BaseModule*>::iterator it = noreload.find(cmdName); //сначала проверяем словарь неперегружаемых команд
    BaseModule* target;
    if (it != noreload.end())
        target = it->second;
    else
        target = coreIface.where(cmdName);
    target->ask(cmdName, cmdArgs);
    return;
}

void Core::write(const INFO_TYPE& infoType, const std::string& mess)
{
    outputs.push_back({infoType, mess});
}

json Core::collectOut()
{
    json temp;
    for (const auto& e : outputs)
    {
        json entry = { {"type", toStr(e.first)}, {"mess", e.second} };
        temp.push_back(entry);
    }
    outputs.clear();
    return temp;
}

/*****функционал_ядра*****************/
int getUserHashByName(string& name, string& hash)
{
    int match = 0;
    ifstream usersFile("data/users.list");
    string buf, login;
    while (!usersFile.eof()) {
        getline(usersFile, buf);
        login = buf.substr(0, buf.find(' '));
        if (login == name) {
            match = 1;
            hash = buf.substr(buf.find(' ')+1);
            break;
        }
    }
    usersFile.close();
    return match;
}
int getUserHashByName(string& name)
{
    string temp;
    return getUserHashByName(name, temp);
}
void addUserAcc(string name, string hash)
{
    string put = name + " " + hash;
    ofstream usersFile("data/users.list", ios_base::app);
    usersFile << endl << put;
    usersFile.close();
    return;
}
void Core::logIn(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?")
        {
            write(INFO_TYPE::TXT, "<вход> - Представиться системе. \
Также возможен ограниченный анонимный доступ.");
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"вход"<<endl;
            return;
        }
    }

    switch (cmdArgs.size())
    {
        //Запрос имени пользователя
        case 0:
        {
            json u = { {"user-name", user()} };
            write(INFO_TYPE::ANCL, u.dump());
            return;
        }
        //Проверка существования данного имени пользователя
        case 1:
        {
            int found = getUserHashByName(cmdArgs[0]);
            stringstream ss; ss << found;

            write(INFO_TYPE::ANCL, ss.str());
            return;
        }
        //Что-то ни то ни сё
        case 2:
        {
            json u = { {"login-error",
                        "Неверный формат данных для входа."} };
            write(INFO_TYPE::ANCL, u.dump());
            return;
        }
        //Регистрация или вход (гарантированно >=3 аргументов)
        default:
        {
            if (!(cmdArgs[0] == "0" || cmdArgs[0] == "1"))
            {
                json u = { {"login-error",
                            "Неверный формат данных для входа."} };
                write(INFO_TYPE::ANCL, u.dump());
                return;
            }
            //Вход
            if (cmdArgs[0] == "0")
            {
                string hashFormBase;
                int found = getUserHashByName(cmdArgs[1], hashFormBase);
                if (found == 0)
                {
                    json u = { {"login-error",
                                "Неверный логин или пароль."} };
                    write(INFO_TYPE::ANCL, u.dump());
                    return;
                }
                else
                {
                    if (cmdArgs[2] != hashFormBase)
                    {
                        json u = { {"login-error",
                                    "Неверный логин или пароль."} };
                        write(INFO_TYPE::ANCL, u.dump());
                        return;
                    }
                    else
                    {
                        userName = cmdArgs[1];
                        json u = { {"user-name", user()} };
                        write(INFO_TYPE::ANCL, u.dump());
                        return;
                    }
                }
            }
            //Регистрация
            else
            {
                int found = getUserHashByName(cmdArgs[1]);
                if (found == 1)
                {
                    json u = { {"login-error",
                                "Пользователь с таким именем уже сществует."} };
                    write(INFO_TYPE::ANCL, u.dump());
                    return;
                }
                else
                {
                    addUserAcc(cmdArgs[1], cmdArgs[2]);
                    userName = cmdArgs[1];
                    json u = { {"user-name", user()} };
                    write(INFO_TYPE::ANCL, u.dump());
                    return;
                }
            }
        }
    }
}

void Core::logOut(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            write(INFO_TYPE::TXT, "<выход> - Вернуться к анонимности.");
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"выход"<<endl;
            return;
        }
    }
    
    userName = "?";
    json u = { {"user-name", user()} };
    write(INFO_TYPE::ANCL, u.dump());
    return;
}

void Core::end(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            write(INFO_TYPE::TXT, "<конец> - Завершение работы программы.");
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"конец"<<endl;
            return;
        }
    }
    
    userName = "?";
    
    throw exit_ex();
}

void Core::plugIn(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            write(INFO_TYPE::TXT, "<подключить [список имён]> - \
Загрузить модули с именами из списка.");
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"подключить"<<endl;
            return;
        }
    }
    
    if (cmdArgs.size() == 0)
    {
        write(INFO_TYPE::TXT, "Пропущено имя плагина.");
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
        write(INFO_TYPE::TXT, "Возникла ошибка при попытке подключения плагина " +
                pluginFullName + ":\n" + ex.what() + "\n");
    }
    ifaceRefresh();
    // слипания заголовков не произойдет,
    // т.к. новый вызов write порождает новую пару вывода
    getInterface({});
    return;
}

void Core::plugOut(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            write(INFO_TYPE::TXT, "<отключить [список имён]> - \
Выгрузить модули с именами из списка.");
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"отключить"<<endl;
            return;
        }
    }
    
    string pluginFullName = "./plugins/"+cmdArgs[0]+".so";
    auto it = SO_inWork.find(pluginFullName);
    if (it != SO_inWork.end()) {
        delete it->second;
        SO_inWork.erase(it);
    }
    ifaceRefresh();
    getInterface({});
    return;
}

void Core::getInterface(std::vector<std::string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?")
        {
            write(INFO_TYPE::TXT, "<getInterface> - \
Служебная команда для запроса словаря доступных вызовов.");
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout << "getInterface"<<endl;
            return;
        }
    }

    map<string, string> iFace = coreIface.getIface();
    json j = {"interface", iFace};
    write(INFO_TYPE::ANCL, j.dump());
}

//todo На стороне сервера: при подключении плагина <помощь> пополняется.
void Core::printListOfComands()
{
    write(INFO_TYPE::TXT, "Список доступных команд:\n\
<вход>\n\t- Представиться системе.\n\
\tТакже возможен ограниченный анонимный доступ.\n\
<выход>\n\t- Вернуться к анонимности.\n\
<запись>\n\t- Сделать запись в личном дневнике.\n\
<конец>\n\t- Завершение работы программы.\n\
<отключить [список имён]>\n\t- Выгрузить модули из списка.\n\
<подключить [список имён]>\n\t- Чтобы загрузить модули из списка,\n\tдоступно автодополнение.\n\
<помощь [имя команды]>\n\t- Вывод справки о команде.\n\
<править [имя модуля]>\n\t- Для изменения модуля (в том числе подключенного).\n");
    return;
}

void Core::getMan(vector<string> cmdArgs)
{ 
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            write(INFO_TYPE::TXT, "<помощь [команда/модуль]> - \
Вывод справки о команде/модуле.");
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"помощь"<<endl;
            return;
        }
    }
    
    if (cmdArgs.size() == 0)
        printListOfComands();
    else
        call(cmdArgs[0],vector<string>({"?"}));
    return;
}