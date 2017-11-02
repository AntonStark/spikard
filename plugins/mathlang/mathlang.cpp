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
    Section* storage;
    Section* current;
    void resetStorage(Section* _storage);
    string userCheck();
    void printIncr(Section* source);

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
    virtual ~MathlangPlugin() { delete storage; }
    virtual void destroy() override
    { fabric->destroy(this); }

    // из-за того, что methods имеет вторым параметром указатель на функцию-член, его
    // нельзя вынести в BaseModule, а следовательно и ask(...)
    virtual void ask(string cmdName, vector<string> cmdArgs) override
    { (this->*methods[cmdName])(cmdArgs); }
    virtual void ifaceCfg() override;
    virtual void write(const INFO_TYPE&, const std::string& mess) override;
};

void MathlangPlugin::resetStorage(Section* _storage)
{
    delete storage;
    storage = _storage;
    current = storage;
}

#define funcInfo(cmd, help) \
{\
    if (cmdArgs.size() > 0)\
    {\
        if (cmdArgs[0] == "?")\
        {\
	    write(INFO_TYPE::TXT, help); return;\
        }\
        else if (cmdArgs[0] == "*")\
        {\
	    cout << cmd << endl; return;\
        }\
    }\
}
//todo На стороне сервера: при подключении плагина <помощь> пополняется.
void MathlangPlugin::printIncr(Section* source)
{
    std::list<std::string> buf;
    source->printMlObjIncr(buf);
    for (const auto& l : buf)
        write(INFO_TYPE::ML_OBJ, l);
}
void MathlangPlugin::addType(vector<string> cmdArgs)
{
    funcInfo("тип", "<тип [typeName]> - ввести тип typeName.")

    if (cmdArgs.size() < 1)
        return;
    try {
        current->defType(cmdArgs[0]);
        printIncr(current);
    }
    catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
}

void MathlangPlugin::addSym(vector<string> cmdArgs)
{
    funcInfo("символ",
             "<символ [symName] [argT]* [retT]> - ввести символ symName.")

    if (cmdArgs.size() < 2)
        return;
    list<string> argTypes(next(cmdArgs.begin()), prev(cmdArgs.end()));
    try {
        current->defSym(cmdArgs.front(), argTypes, cmdArgs.back());
        printIncr(current);
    }
    catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
}

void MathlangPlugin::addVar(vector<string> cmdArgs)
{
    funcInfo("переменная",
             "<переменная [varName] [varType]> - ввести переменную varName типа varType.")

    if (cmdArgs.size() < 2)
        return;
    try {
        current->defVar(cmdArgs[0], current->index().getT(cmdArgs[1]).getName());
        printIncr(current);
    }
    catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
}

void MathlangPlugin::addAxiom(vector<string> cmdArgs)
{
    funcInfo("пусть", "<пусть [statement]> - ввести утверждение statement.")

    if (cmdArgs.size() < 1)
        return;
    current->addAxiom(cmdArgs[0]);
    printIncr(current);
}


void MathlangPlugin::viewTypes(vector<string> cmdArgs)
{
    funcInfo("типы", "<типы> - перечислить уже определённые типы.")

    write(INFO_TYPE::TXT, "Опеределены следующие типы:");
    for (const auto& n : current->index().getNames(NameTy::MT))
        write(INFO_TYPE::TXT, n);
}

void MathlangPlugin::viewSyms(vector<string> cmdArgs)
{
    funcInfo("символы", "<символы> - перечислить уже определённые символы.")

    write(INFO_TYPE::TXT, "Опеределены следующие имена символов:");
    for (const auto& n : current->index().getNames(NameTy::SYM))
        write(INFO_TYPE::TXT, n);
}

void MathlangPlugin::viewVars(vector<string> cmdArgs)
{
    funcInfo("переменные", "<переменные> - перечислить уже определённые переменные.")

    write(INFO_TYPE::TXT, "Опеределены следующие имена переменных:");
    for (const auto& n : current->index().getNames(NameTy::VAR))
        write(INFO_TYPE::TXT, n);
}

void MathlangPlugin::subSection(vector<string> cmdArgs)
{
    funcInfo("подраздел",
             "<подраздел [название(необязательно)]> - начать подраздел с данным названием.")

    if (cmdArgs.size() > 0)
        current->startSection(cmdArgs[0]);
    else
        current->startSection();
}

void MathlangPlugin::gotoSection(vector<string> cmdArgs)
{
    funcInfo("перейти",
             "<перейти [PathToSection]> - перейти к Section с меткой PathToSection.")

    if (cmdArgs.size() < 1)
        return;

    Section* target = current->getSub(cmdArgs[0]);
    if (target)
        current = target;
    printIncr(current);
}

void MathlangPlugin::viewSection(vector<string> cmdArgs)
{
    funcInfo("показать_рассуждение",
             "<показать_рассуждение> - показать рассуждение целиком.")
    printIncr(current);
}

string MathlangPlugin::userCheck()
{
    BaseModule* parent = this->getParent();
    while (BaseModule* p = parent->getParent())
        parent = p;
    Core* core;
    if (!(core = dynamic_cast<Core*>(parent)))
    {
        write(INFO_TYPE::TXT, "Ошибка: сохранение не удалось по внутрненним причинам.");
        return "";
    }

    string userName = core->user();
    if (userName == "?")
    {
        write(INFO_TYPE::TXT, "Ошибка: сохранение и загрузка недоступны в анонимном режиме.");
        return "";
    }
    return userName;
}
void MathlangPlugin::saveSection(vector<string> cmdArgs)
{
    funcInfo("сохранить",
                 "<сохранить> - запись для последующего использования.")

    string userName = userCheck();
    if (userName.empty())
        return;

    string fileName = "/home/anton/development/spikard/users/" + userName +
            "/data/mathlang/section/" + current->getTitle();
    ofstream osf(fileName);
    if (!osf.is_open())
        write(INFO_TYPE::TXT, "Ошибка: не удалось создать файл.");
    osf << current->toJson().dump(2) << endl;
    write(INFO_TYPE::TXT, "Сохранено.");
}

void MathlangPlugin::loadSection(vector<string> cmdArgs)
{
    funcInfo("загрузить", "<загрузить> - открыть ранее сохраненное.")

    if (cmdArgs.size() < 1)
        return;

    string userName = userCheck();
    if (userName.length() == 0)
        return;

    string fileName = "/home/anton/development/spikard/users/" + userName +
            "/data/mathlang/section/" + cmdArgs[0];
    ifstream isf(fileName);
    if (!isf.is_open())
        write(INFO_TYPE::TXT, "Ошибка: не удалось открыть файл.");

    stringstream buf;
    buf << isf.rdbuf();
    json j = json::parse(buf.str());
    HierarchyItem* read = Section::fromJsonE(j);
    if (Section* s = dynamic_cast<Section*>(read))
        resetStorage(s);
    printIncr(current);
}

void MathlangPlugin::deduceMP(vector<string> cmdArgs)
{
    funcInfo("MP",
             "<MP [PathPre] [PathImpl]> - применить правило вывода MP для Pre и Impl.")

    if (cmdArgs.size() < 2)
        return;
    current->doMP(cmdArgs[0], cmdArgs[1]);
    printIncr(current);
}

void MathlangPlugin::deduceSpec(vector<string> cmdArgs)
{
    funcInfo("spec",
             "<spec [PathGen] [PathT]> - применить правило вывода Spec для Gen и T.")

    if (cmdArgs.size() < 2)
        return;
    current->doSpec(cmdArgs[0], cmdArgs[1]);
    printIncr(current);
}

void MathlangPlugin::deduceGen(vector<string> cmdArgs)
{
    funcInfo("gen",
             "<gen [PathToGen] [PathVar]> - применить правило вывода Gen для ToGen и Var.")

    if (cmdArgs.size() < 2)
        return;
    current->doGen(cmdArgs[0], cmdArgs[1]);
    printIncr(current);
}

MathlangPlugin::MathlangPlugin(BaseModule* _parent, SharedObject* _fabric)
: BaseModule(ModuleInfo("Теория типов", "0.3", "04.07.17"), _parent)
{
    // Инициализация переменных здесь
    storage = new Section("Главный");
    current = storage;

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

void MathlangPlugin::write(const INFO_TYPE& type, const std::string& mess)
{
    // todo Написать Core* BaseModule::getRoot()
    BaseModule* mod = this;
    while (mod->getParent())
        mod = mod->getParent();
    Core* root = static_cast<Core*>(mod);

    return root->write(type, mess);
}

extern "C" BaseModule* create(BaseModule* _parent, SharedObject* _fabric)
{ return new MathlangPlugin(_parent, _fabric); }

extern "C" void destroy(BaseModule* one)
{
    delete one;
    return;
}
