#ifndef CORE_H
#define CORE_H

#include <algorithm>
#include <dlfcn.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <initializer_list>
#include <list>
#include <map>
#include <set>
#include <string>
#include <termios.h>
#include <vector>
#include <sstream>

#include "json.hpp"
using json = nlohmann::json;

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

struct ModuleInfo {
    std::string name;
    std::string version;
    std::string timeOfLastChange;

    ModuleInfo(std::string _name, std::string _ver, std::string _time) :
            name(_name), version(_ver), timeOfLastChange(_time) { }
};

// описывает типы информации в потоках вывода
enum class INFO_TYPE {ANCL, TXT, TEX_EXP, ML_OBJ, ERR, NAME};
std::string toStr(const INFO_TYPE& infoType);

class BaseModule {
private:
    const ModuleInfo info;
    BaseModule *const parent;
    std::list<BaseModule *> modules;
    //std::list<BaseModule*> dependensies;
public:
    BaseModule(ModuleInfo _info, BaseModule *_parent = NULL) :
            info(_info), parent(_parent) {
        modules.push_front(this);
    }

    virtual void destroy() {//нужно для корректного уничтожения модулей
        delete this;
    }

    virtual ~BaseModule() {
        while (modules.size() > 1)
            modules.back()->destroy();
    }

    ModuleInfo getModuleInfo() const { return info; }

    BaseModule* const getParent() const { return parent; }
    BaseModule* getRoot() {
        BaseModule* mod = this;
        while (mod->getParent())
            mod = mod->getParent();
        return mod;
    }

    void registerModule(BaseModule *some) {
        modules.push_back(some);
        return;
    }

    void deregisterModule(BaseModule *some) {
        auto it = modules.begin();
        while (*it != some && it != modules.end())
            ++it;
        if (*it == some)
            modules.erase(it);
        return;
    }

    //bool find(std::string, std::vector<std::string>);
    virtual void ask(std::string, std::vector<std::string>) = 0;

    void ifaceRefresh();

    virtual void ifaceCfg() = 0;
    virtual void write(const INFO_TYPE&, const std::string& mess) = 0;
};

class SharedObject {
private:
    void *libHandle;
    std::set<BaseModule *> items;

    BaseModule *(*createPlugin)(BaseModule *, void *);

    void (*destroyPlugin)(BaseModule *);

    SharedObject() = delete;

    SharedObject(const SharedObject &) = delete;

    SharedObject &operator=(const SharedObject &) = delete;

public:
    SharedObject(const std::string &, int);

    ~SharedObject();

    BaseModule *create(BaseModule *);

    void destroy(BaseModule *);
};

class IFace {
private:
    std::map<std::string, BaseModule *> index;
    std::map<std::string, std::string> iface;
public:
    void add(std::string, std::string, BaseModule *);

    void clear();

    BaseModule *where(std::string);

    std::string operator[](std::string);

    std::map<std::string, std::string> getIface() const;
};

class Core;

typedef void (Core::*pCoreHandler)(std::vector<std::string>);

class Core : public BaseModule {
private:
    std::string userName;

    void logIn(std::vector<std::string>);

    void logOut(std::vector<std::string>);

    void end(std::vector<std::string>);

    void plugIn(std::vector<std::string>);

    void plugOut(std::vector<std::string>);

    void getInterface(std::vector<std::string>);

    void printListOfComands();

    void getMan(std::vector<std::string>);

    std::map<std::string, pCoreHandler> methods;

    void methodsCfg();

    void ifaceCfg();

    std::map<std::string, BaseModule *> noreload;

    //защита от того, что команды будут перeгружены новыми плагинами
    void noreload_init();

    std::map<std::string, SharedObject *> SO_inWork;

    //словарь каналов вывода по типам информации
    std::vector<std::pair<INFO_TYPE, std::string> > outputs;
public:
    Core();

    ~Core() {
        while (!SO_inWork.empty()) {
            delete SO_inWork.begin()->second;
            SO_inWork.erase(SO_inWork.begin());
        }
    }

    void ask(std::string, std::vector<std::string>);

    void call(std::string, std::vector<std::string>);

    void write(const INFO_TYPE&, const std::string& mess) override;
    json collectOut();

    std::string user() const { return userName; }

    IFace coreIface;
};

#define CALL_INFO(cmd, help) \
{\
    if (!cmdArgs.empty())\
    {\
        if (cmdArgs[0] == "?")\
        {\
	    write(INFO_TYPE::TXT, help); return;\
        }\
        else if (cmdArgs[0] == "*")\
        {\
	    cout << (cmd) << endl; return;\
        }\
    }\
}

#endif