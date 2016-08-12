#ifndef CORE_H
#define CORE_H

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <termios.h>
#include <list>
#include <dlfcn.h>
#include <exception>
#include <set>
#include <initializer_list>
#include <algorithm>

struct ModuleInfo
{
    std::string name;
    std::string version;
    std::string timeOfLastChange;
    ModuleInfo(std::string _name, std::string _ver, std::string _time) :
        name(_name), version(_ver), timeOfLastChange(_time) {}
};

class BaseModule
{
private:
    const ModuleInfo info;
    BaseModule* const parent;
    std::list<BaseModule*> modules;
    //std::list<BaseModule*> dependensies;
public:
    BaseModule(ModuleInfo _info, BaseModule* _parent = NULL) :
        info(_info), parent(_parent)
    {
        modules.push_front(this);
    }
    virtual void destroy() {//нужно для корректного уничтожения модулей
        delete this;
    }
    virtual ~BaseModule()
    {
        while (modules.size() > 1)
            modules.back()->destroy();
    }

    ModuleInfo getModuleInfo() const;
    BaseModule* const getParent() const {return parent;}
    bool find(std::string, std::vector<std::string>);
    virtual bool ask(std::string, std::vector<std::string>) = 0;
    void registerModule(BaseModule* some)
    {
        modules.push_back(some);
        return;
    }
    void deregisterModule(BaseModule* some)
    {
        auto it = modules.begin();
        while (*it != some && it != modules.end())
            ++it;
        if (*it == some)
            modules.erase(it);
        return;
    }
};

class SharedObject
{
private:
    void* libHandle;
    std::set<BaseModule*> items;
    BaseModule* (*createPlugin)(BaseModule*, void*);
    void (*destroyPlugin)(BaseModule*);

    SharedObject() = delete;
    SharedObject(const SharedObject&) = delete;
    SharedObject &operator= (const SharedObject&) = delete;
public:
    SharedObject(const std::string&, int);
    ~SharedObject();

    BaseModule* create(BaseModule*);
    void destroy(BaseModule*);
};

class Core;
typedef void (Core::*pCoreHandler)(std::vector<std::string>);
class Core : public BaseModule
{
private:
    std::string userName;
    void emptyComand(std::vector<std::string>);
    void logIn(std::vector<std::string>);
    void logOut(std::vector<std::string>);
    void diary(std::vector<std::string>);
    void end(std::vector<std::string>);
    void plugIn(std::vector<std::string>);
    void printListOfComands(std::vector<std::string>);
    void change(std::vector<std::string>);
    void getMan(std::vector<std::string>);

    std::map<std::string, pCoreHandler> connect;
    std::map<std::string, std::string> mans;
    void add(std::string cmd, pCoreHandler pFun, std::string man) 
    {
        connect.insert(std::make_pair(cmd, pFun));
        mans.insert(std::make_pair(cmd, man));
        return;
    }
    std::map<std::string, BaseModule*> noreload;//защита от того, что команды будут перeгружены новыми плагинами
    void noreload_init();
    std::map<std::string, SharedObject*> SO_inWork;
public:
    Core(std::string);
    ~Core()
    {
        while (!SO_inWork.empty())
        {
            delete SO_inWork.begin()->second;
            SO_inWork.erase(SO_inWork.begin());
        }
    }

    bool ask(std::string, std::vector<std::string>);
    void call(std::string, std::vector<std::string>);
    std::string user() const {return userName;}
};


#include "core.cpp"
#endif
