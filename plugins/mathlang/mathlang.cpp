//
// Created by anton on 04.07.17.
//

#include "../../core.hpp"
#include "signature.hpp"

using namespace std;

class MathlangPlugin : public BaseModule
{
private:
    // Здесь описываются необходимые переменные
    Reasoning reasoning;

    // Далее следуют функции, реализующие функционал плагина
    void addType(vector<string> cmdArgs);
    void addSym(vector<string> cmdArgs);
    void addVar(vector<string> cmdArgs);
    void addStatement(vector<string> cmdArgs);

    void viewTypes(vector<string> cmdArgs);
    void viewSyms(vector<string> cmdArgs);
    void viewVars(vector<string> cmdArgs);

    map<string, void (MathlangPlugin::*)(vector<string>)> methods;
    void methodsCfg();
    SharedObject* fabric;
public:
    MathlangPlugin(BaseModule*, SharedObject*);
    virtual ~MathlangPlugin() {}
    virtual void destroy() override
    { fabric->destroy(this); }

    virtual void ask(string cmdName, vector<string> cmdArgs) override
    { (this->*methods[cmdName])(cmdArgs); }
    virtual void ifaceCfg() override;
};


void MathlangPlugin::addType(vector<string> cmdArgs)
{
    if (cmdArgs.size() != 0)
    {
        if (cmdArgs[0] == "?")
        {
            cout<<"<новый_тип [typeName]> - ввести тип typeName."<<endl;
            return;
        }
        else if (cmdArgs[0] == "*")
        {
            cout<<"новый_тип"<<endl;
            return;
        }
    }

    reasoning.addType(cmdArgs[0]);
}


MathlangPlugin::MathlangPlugin(BaseModule* _parent, SharedObject* _fabric)
: BaseModule(ModuleInfo("Теория типов", "0.3", "04.07.17"), _parent)
{
    // Инициализация переменных здесь

    methodsCfg();
    fabric = _fabric;
}

void MathlangPlugin::methodsCfg()
{
    methods.insert(make_pair("add_type", &MathlangPlugin::addType));

    // Сюда добавлять функционал плагина
}

void MathlangPlugin::ifaceCfg()
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

extern "C" BaseModule* create(BaseModule* _parent, SharedObject* _fabric)
{ return new MathlangPlugin(_parent, _fabric); }

extern "C" void destroy(BaseModule* one)
{
    delete one;
    return;
}