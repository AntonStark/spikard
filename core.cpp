using namespace std;

ModuleInfo coreInfo("Феникс", "Фи", "30.01.16");

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
    if (auto fn = index.find(funName) != index.end() )
        return;
    if (auto kw = iface.find(keyWord) != iface.end() )
        keyWord = keyWord + "_in_" + contain->getModuleInfo().name;
    index.insert(make_pair(funName, contain));
    iface.insert(make_pair(keyWord, funName));
}

void IFace::clear()
{
    index.clear();
    iface.clear();
}

BaseModule* IFace::where(std::string funName)
{
    return index[funName];
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

/*****функционал_ядра*****************/
void Core::logIn(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            cout<<"<вход> - Представиться системе. \
Также возможен ограниченный анонимный доступ."<<endl;
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"вход"<<endl;
            return;
        }
    }
    
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

void Core::logOut(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            cout<<"<выход> - Вернуться к анонимности."<<endl;
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"выход"<<endl;
            return;
        }
    }
    
    userName = "?";
    return;
}

void Core::end(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            cout<<"<конец> - Завершение работы программы."<<endl;
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
    return;
}

void Core::plugIn(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            cout<<"<подключить [список имён]> - \
Загрузить модули с именами из списка."<<endl;
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
    ifaceRefresh();
    return;
}

void Core::plugOut(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            cout<<"<отключить [список имён]> - \
Выгрузить модули с именами из списка."<<endl;
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
    return;
}

void Core::printListOfComands()
{
    cout<<"Список доступных команд:"<<endl;
    cout<<"<вход>\n\t- Представиться системе."<<endl;
    cout<<"\tТакже возможен ограниченный анонимный доступ."<<endl;
    cout<<"<выход>\n\t- Вернуться к анонимности."<<endl;
    cout<<"<запись>\n\t- Сделать запись в личном дневнике."<<endl;
    cout<<"<конец>\n\t- Завершение работы программы."<<endl;
    cout<<"<отключить [список имён]>\n\t- Выгрузить модули из списка."<<endl;
    cout<<"<подключить [список имён]>\n\t- Чтобы загрузить модули из списка,\n\tдоступно автодополнение."<<endl;
    cout<<"<помощь [имя команды]>\n\t- Вывод справки о команде."<<endl;
    cout<<"<править [имя модуля]>\n\t- Для изменения модуля (в том числе подключенного)."<<endl;
    return;
}

void Core::getMan(vector<string> cmdArgs)
{ 
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?") 
        {
            cout<<"<помощь [команда/модуль]> - \
Вывод справки о команде/модуле."<<endl;
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