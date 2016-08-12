#ifndef CORE_H
#define CORE_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <termios.h>

struct ModuleInfo {
    std::string name;
    std::string version;
};

class BaseModule {
    ModuleInfo info;
public:
    ModuleInfo getModuleInfo() const;
    virtual ~BaseModule() {}
};

class Core : public BaseModule {
    std::map<std::string, std::string> mans;

    static void emptyComand(std::vector<std::string>);
    static void logIn(std::vector<std::string>);
    static void logOut(std::vector<std::string>);
    static void diary(std::vector<std::string>);
    static void end(std::vector<std::string>);
    static void plugIn(std::vector<std::string>);
    static void printListOfComands(std::vector<std::string>);
    static void change(std::vector<std::string>);
    void getMan(std::vector<std::string>);
public:
    Core(std::string);
    ~Core();

    std::map<std::string, void (*)(std::vector<std::string>)> connect;
    static std::string userName;
    static bool run;
};

#endif
