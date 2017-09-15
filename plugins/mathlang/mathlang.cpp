//
// Created by anton on 04.07.17.
//

#include "../../core.hpp"
#include "signature.hpp"
#include "parser.hpp"

using namespace std;

class MathlangPlugin : public BaseModule
{
private:
    // Здесь описываются необходимые переменные
    Section storage;
    Section* current;

    // Далее следуют функции, реализующие функционал плагина
    void addType(vector<string> cmdArgs);
    void addSym(vector<string> cmdArgs);
    void addVar(vector<string> cmdArgs);
    void addAxiom(vector<string> cmdArgs);

    void viewTypes(vector<string> cmdArgs);
    void viewSyms(vector<string> cmdArgs);
    void viewVars(vector<string> cmdArgs);

    void subSection (vector<string> cmdArgs);
    void gotoSection(vector<string> cmdArgs);
    void viewSection(vector<string> cmdArgs);
    void saveSection(vector<string> cmdArgs);
    void loadSection(vector<string> cmdArgs);

    void deduceMP(vector<string> cmdArgs);
    void deduceSpec(vector<string> cmdArgs);
    void deduceGen(vector<string> cmdArgs);

    map<string, void (MathlangPlugin::*)(vector<string>)> methods;
    void methodsCfg();
    SharedObject* fabric;
public:
    MathlangPlugin(BaseModule*, SharedObject*);
    virtual ~MathlangPlugin() {}
    virtual void destroy() override
    { fabric->destroy(this); }

    // из-за того, что methods имеет вторым параметром указатель на функцию-член, его
    // нельзя вынести в BaseModule, а следовательно и ask(...)
    virtual void ask(string cmdName, vector<string> cmdArgs) override
    { (this->*methods[cmdName])(cmdArgs); }
    virtual void ifaceCfg() override;
};

bool funcInfo(const vector<string>& cmdArgs, string cmd, string help)
{
    if (cmdArgs.size() == 0)
        return false;

    if (cmdArgs[0] == "?")
    {
        cout << help << endl; return true;
    }
    else if (cmdArgs[0] == "*")
    {
        cout << cmd  << endl; return true;
    }
    else
        return false;
}
//todo На стороне сервера: при подключении плагина <помощь> пополняется.
void MathlangPlugin::addType(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "тип",
                 "<тип [typeName]> - ввести тип typeName."))
        return;

    if (cmdArgs.size() < 1)
        return;
    try {
        current->defType(cmdArgs[0]);
        current->printB(cout);
    }
    catch (std::exception& e) { cout << "Ошибка: " << e.what() << endl; }
}

void MathlangPlugin::addSym(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "символ",
             "<символ [symName] [argT]* [retT]> - ввести символ symName."))
        return;

    if (cmdArgs.size() < 2)
        return;
    list<string> argTypes(next(cmdArgs.begin()), prev(cmdArgs.end()));
    try {
        current->defSym(cmdArgs.front(), argTypes, cmdArgs.back());
        current->printB(cout);
    }
    catch (std::exception& e) { cout << "Ошибка: " << e.what() << endl; }
}

void MathlangPlugin::addVar(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "переменная",
             "<переменная [varName] [varType]> - ввести переменную varName типа varType."))
        return;

    if (cmdArgs.size() < 2)
        return;
    try {
        current->defVar(cmdArgs[0], current->index().getT(cmdArgs[1]).getName());
        current->printB(cout);
    }
    catch (std::exception& e) { cout << "Ошибка: " << e.what() << endl; }
}

void MathlangPlugin::addAxiom(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "пусть",
             "<пусть [statement]> - ввести утверждение statement."))
        return;

    if (cmdArgs.size() < 1)
        return;
    current->addAxiom(cmdArgs[0]);
    current->printB(cout);
}


void MathlangPlugin::viewTypes(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "типы",
             "<типы> - перечислить уже определённые типы."))
        return;

    for (auto& b : current->index().getNames(NameTy::MT))
        cout << b << endl;
}

void MathlangPlugin::viewSyms(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "символы",
             "<символы> - перечислить уже определённые символы."))
        return;

    for (auto& b : current->index().getNames(NameTy::SYM))
        cout << b << endl;
}

void MathlangPlugin::viewVars(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "переменные",
             "<переменные> - перечислить уже определённые переменные."))
        return;

    for (auto& b : current->index().getNames(NameTy::VAR))
        cout << b << endl;
}

void MathlangPlugin::subSection(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "подраздел",
                 "<подраздел [название(необязательно)]> - начать подраздел с данным названием."))
        return;

    if (cmdArgs.size() > 0)
        current->startSection(cmdArgs[0]);
    else
        current->startSection("");
}

void MathlangPlugin::gotoSection(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "перейти",
                 "<перейти [PathToSection]> - перейти к Section с меткой PathToSection."))
        return;

    if (cmdArgs.size() < 1)
        return;

    Section* target = current->getSub(cmdArgs[0]);
    if (target)
        current = target;
    current->printB(cout);
}

void MathlangPlugin::viewSection(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "показать_рассуждение",
                 "<показать_рассуждение> - показать рассуждение целиком."))
        return;
    current->printB(cout);
}

void MathlangPlugin::saveSection(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "сохранить_рассуждение",
                 "<сохранить_рассуждение> - запись для последующего использования."))
        return;
}

void MathlangPlugin::loadSection(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "загрузить_рассуждение",
                 "<загрузить_рассуждение> - открыть ранее сохраненное."))
        return;
}

void MathlangPlugin::deduceMP(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "MP",
             "<MP [PathPre] [PathImpl]> - применить правило вывода MP для Pre и Impl."))
        return;

    if (cmdArgs.size() < 2)
        return;
    current->doMP(cmdArgs[0], cmdArgs[1]);
    current->printB(cout);
}

void MathlangPlugin::deduceSpec(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "spec",
             "<spec [PathGen] [PathT]> - применить правило вывода Spec для Gen и T."))
        return;

    if (cmdArgs.size() < 2)
        return;
    current->doSpec(cmdArgs[0], cmdArgs[1]);
    current->printB(cout);
}

void MathlangPlugin::deduceGen(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "gen",
                 "<gen [PathToGen] [PathVar]> - применить правило вывода Gen для ToGen и Var."))
        return;

    if (cmdArgs.size() < 2)
        return;
    current->doGen(cmdArgs[0], cmdArgs[1]);
    current->printB(cout);
}

MathlangPlugin::MathlangPlugin(BaseModule* _parent, SharedObject* _fabric)
: BaseModule(ModuleInfo("Теория типов", "0.3", "04.07.17"), _parent), storage("Главный")
{
    // Инициализация переменных здесь
    current = &storage;

    methodsCfg();
    fabric = _fabric;
}

void MathlangPlugin::methodsCfg()
{
    methods.insert(make_pair("add_type" , &MathlangPlugin::addType));
    methods.insert(make_pair("add_sym"  , &MathlangPlugin::addSym));
    methods.insert(make_pair("add_var"  , &MathlangPlugin::addVar));
    methods.insert(make_pair("add_axiom", &MathlangPlugin::addAxiom));

    methods.insert(make_pair("view_types", &MathlangPlugin::viewTypes));
    methods.insert(make_pair("view_syms", &MathlangPlugin::viewSyms));
    methods.insert(make_pair("view_vars", &MathlangPlugin::viewVars));

    methods.insert(make_pair("sub_section",  &MathlangPlugin::subSection));
    methods.insert(make_pair("goto_section", &MathlangPlugin::gotoSection));
    methods.insert(make_pair("view_section", &MathlangPlugin::viewSection));
    methods.insert(make_pair("save_section", &MathlangPlugin::saveSection));
    methods.insert(make_pair("load_section", &MathlangPlugin::loadSection));

    methods.insert(make_pair("deduce_MP", &MathlangPlugin::deduceMP));
    methods.insert(make_pair("deduce_Spec", &MathlangPlugin::deduceSpec));
    methods.insert(make_pair("deduce_Gen", &MathlangPlugin::deduceGen));
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
