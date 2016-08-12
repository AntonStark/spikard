#ifndef CORE_H
#define CORE_H

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <termios.h>
#include <list>

class AbstractMap
{
public:
    AbstractMap() {}
    virtual ~AbstractMap() {}
};

template<class Module>
class TermMap : public AbstractMap
{
public:
    TermMap() {}
    ~TermMap() {}
    std::map<std::string, void (Module::*)(std::vector<std::string>)> connect;
    std::map<std::string, std::string> mans;

    void add(std::string cmd, void (Module::*pFun)(std::vector<std::string>), std::string man) {
        connect.insert(std::make_pair(cmd, pFun));
        mans.insert(std::make_pair(cmd, man));
        return;
    }
};
/*
template<class Module>
class GraphicMap : public AbstractMap
{
public:
    GraphicMap() {}
    ~GraphicMap() {}
    //std::map< , void (Module::*)()> connect;
    //std::map< , > mans;
};

template<class Module>
class GeneralMap : public TermMap<Module>, public GraphicMap<Module>
{
};
*/

struct ModuleInfo
{
    std::string name;
    std::string version;
};

class BaseModule
{
private:
    ModuleInfo info;
public:
    ModuleInfo getModuleInfo() const;
    virtual ~BaseModule() {}
    virtual void call(std::string, std::vector<std::string>) = 0;
};

class Core : public BaseModule
{
private:
    std::string userName;
    bool run;

    void emptyComand(std::vector<std::string>);
    void logIn(std::vector<std::string>);
    void logOut(std::vector<std::string>);
    void diary(std::vector<std::string>);
    void end(std::vector<std::string>);
    void plugIn(std::vector<std::string>);
    void printListOfComands(std::vector<std::string>);
    void change(std::vector<std::string>);
    void getMan(std::vector<std::string>);
    //friend void logIn(std::vector<std::string>);
    //friend void logOut(std::vector<std::string>);

    TermMap<Core> interface;
    std::list<BaseModule*> modules;
public:
    Core(std::string);
    ~Core();

    void call(std::string, std::vector<std::string>);
    bool work() const {return run;}
    std::string user() const {return userName;}
};
#include "core.cpp"
#endif
