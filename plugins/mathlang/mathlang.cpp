//
// Created by anton on 04.07.17.
//

#include "../../core.hpp"
#include "../../lib/mathlang/rationale.hpp"
#include "../../lib/mathlang/parser.hpp"

using namespace std;

class MathlangPlugin : public BaseModule
{
private:
    // Здесь описываются необходимые переменные
    Node* storage;
    Node* current;
    void resetStorage(Node* _storage);
    string userCheck();
//    void printIncr(Lecture* source);

    // Далее следуют функции, реализующие функционал плагина
    void startCourse (vector<string> cmdArgs);
    void startSection(vector<string> cmdArgs);
    void startLecture(vector<string> cmdArgs);

    void toSuper  (vector<string> cmdArgs);
    void toSubNode(vector<string> cmdArgs);
    void viewNode (vector<string> cmdArgs);
    void saveAll(vector<string> cmdArgs);
    void loadAll(vector<string> cmdArgs);

    void addType(vector<string> cmdArgs);
    void addSym(vector<string> cmdArgs);
    void addVar(vector<string> cmdArgs);
    void addAxiom(vector<string> cmdArgs);

    void viewTypes(vector<string> cmdArgs);
    void viewSyms(vector<string> cmdArgs);
    void viewVars(vector<string> cmdArgs);

    void deduceMP  (vector<string> cmdArgs);
    void deduceSpec(vector<string> cmdArgs);
    void deduceGen (vector<string> cmdArgs);

    map<string, void (MathlangPlugin::*)(vector<string>)> methods;
    void methodsCfg();
    SharedObject* fabric;
public:
    MathlangPlugin(BaseModule*, SharedObject*);
    ~MathlangPlugin() override { delete storage; }
    void destroy() override { fabric->destroy(this); }

    // из-за того, что methods имеет вторым параметром указатель на функцию-член, его
    // нельзя вынести в BaseModule, а следовательно и ask(...)
    void ask(string cmdName, vector<string> cmdArgs) override
    { (this->*methods[cmdName])(cmdArgs); }
    virtual void ifaceCfg() override;
    void write(const INFO_TYPE&, const std::string& mess) override;
};

void MathlangPlugin::resetStorage(Node* _storage)
{
    delete storage;
    storage = _storage;
    current = storage;
}

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
//todo На стороне сервера: при подключении плагина <помощь> пополняется.
/*void MathlangPlugin::printIncr(Lecture* source)
{
    std::list<std::string> buf;
    source->printMlObjIncr(buf);
    for (const auto& l : buf)
        write(INFO_TYPE::ML_OBJ, l);
}*/

void MathlangPlugin::startCourse(vector<string> cmdArgs) {
    CALL_INFO("курс", "<курс [title]> - начать курс с названием title.")

    if (auto* bn = static_cast<BranchNode*>(current)) {
        if (cmdArgs.size() < 1)
            bn->startCourse();
        else
            bn->startCourse(cmdArgs[0]);
        write(INFO_TYPE::TXT, "Курс создан.");
    }
    else
        write(INFO_TYPE::TXT,
              "Нельзя начать курс в первичном узле, перейдите выше.");
}

void MathlangPlugin::startSection(vector<string> cmdArgs) {
    CALL_INFO("раздел", "<раздел [title]> - начать раздел с названием title.")

    if (auto* bn = static_cast<BranchNode*>(current)) {
        if (cmdArgs.size() < 1)
            bn->startSection();
        else
            bn->startSection(cmdArgs[0]);
        write(INFO_TYPE::TXT, "Раздел создан.");
    }
    else
        write(INFO_TYPE::TXT,
              "Нельзя начать раздел в первичном узле, перейдите выше.");
}

void MathlangPlugin::startLecture(vector<string> cmdArgs) {
    CALL_INFO("лекция", "<лекция [title]> - начать лекцию с названием title.")

    if (auto* bn = static_cast<BranchNode*>(current)) {
        if (cmdArgs.size() < 1)
            bn->startLecture();
        else
            bn->startLecture(cmdArgs[0]);
        write(INFO_TYPE::TXT, "Лекция создана.");
    }
    else
        write(INFO_TYPE::TXT,
              "Нельзя начать лекцию в первичном узле, перейдите выше.");
}

void MathlangPlugin::toSuper(vector<string> cmdArgs) {
    CALL_INFO("наверх", "<наверх> - перейти к узлу уровнем выше.")

    Node* target = current->getParent();
    if (target)
        current = target;
}

void MathlangPlugin::toSubNode(vector<string> cmdArgs) {
    CALL_INFO("перейти",
              "<перейти [номер]> - перейти в подраздел с данным порядковым номером.")

    if (cmdArgs.size() < 1)
        return;

    if (auto* bn = static_cast<BranchNode*>(current)) {
        Node* target = static_cast<Node*>(      // subs BranchNode гарантированно типа Node
                bn->getSub( atoi(cmdArgs[0].c_str()) ));
        if (target)
            current = target;
        printIncr(current);
    }
}

void MathlangPlugin::viewNode(vector<string> cmdArgs) {
    CALL_INFO("показать", "<показать> - показать работу целиком.")
    printIncr(current);
}

string MathlangPlugin::userCheck() {
    BaseModule* parent = this->getParent();
    while (BaseModule* p = parent->getParent())
        parent = p;
    Core* core;
    if (!(core = dynamic_cast<Core*>(parent))) {
        write(INFO_TYPE::TXT, "Ошибка: сохранение не удалось по внутрненним причинам.");
        return "";
    }

    string userName = core->user();
    if (userName == "?") {
        write(INFO_TYPE::TXT, "Ошибка: сохранение и загрузка недоступны в анонимном режиме.");
        return "";
    }
    return userName;
}

void MathlangPlugin::saveAll(vector<string> cmdArgs) {
    CALL_INFO("сохранить",
              "<сохранить> - запись для последующего использования.")

    string userName = userCheck();
    if (userName.empty())
        return;

    string fileName = "data/users/" + userName +
                      "/data/mathlang/section/" + current->getName();
    ofstream osf(fileName);
    if (!osf.is_open()) {
        write(INFO_TYPE::TXT, "Ошибка: не удалось создать файл.");
        return;
    }
    osf << current->toJson().dump(2) << endl;
    write(INFO_TYPE::TXT, "Сохранено.");
}

void MathlangPlugin::loadAll(vector<string> cmdArgs) {
    CALL_INFO("загрузить", "<загрузить> - открыть ранее сохраненное.")

    if (cmdArgs.size() < 1)
        return;

    string userName = userCheck();
    if (userName.length() == 0)
        return;

    string fileName = "data/users/" + userName +
                      "/data/mathlang/section/" + cmdArgs[0];
    ifstream isf(fileName);
    if (!isf.is_open()) {
        write(INFO_TYPE::TXT, "Ошибка: не удалось открыть файл.");
        return;
    }

    stringstream buf;
    buf << isf.rdbuf();
    json j = json::parse(buf.str());
    Node* read = Node::fromJsonE(j);
    if (auto* n = dynamic_cast<Node*>(read))
        resetStorage(n);
    printIncr(current);
}

void MathlangPlugin::addType(vector<string> cmdArgs) {
    CALL_INFO("тип", "<тип [typeName]> - ввести тип typeName.")

    if (cmdArgs.size() < 1)
        return;
    if (auto* pn = static_cast<PrimaryNode*>(current)) {
        try {
            pn->defType(cmdArgs[0]);
            printIncr(pn);
        }
        catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
    }
}

void MathlangPlugin::addSym(vector<string> cmdArgs) {
    CALL_INFO("символ",
             "<символ [symName] [argT]* [retT]> - ввести символ symName.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = static_cast<PrimaryNode*>(current)) {
        list<string> argTypes(next(cmdArgs.begin()), prev(cmdArgs.end()));
        try {
            pn->defSym(cmdArgs.front(), argTypes, cmdArgs.back());
            printIncr(pn);
        }
        catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
    }
}

void MathlangPlugin::addVar(vector<string> cmdArgs) {
    CALL_INFO("переменная",
             "<переменная [varName] [varType]> - ввести переменную varName типа varType.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = static_cast<PrimaryNode*>(current)) {
        try {
            pn->defVar(cmdArgs[0], getType(pn->index(), cmdArgs[1]).getName());
            printIncr(pn);
        }
        catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
    }
}

void MathlangPlugin::addAxiom(vector<string> cmdArgs) {
    CALL_INFO("пусть", "<пусть [statement]> - ввести утверждение statement.")

    if (cmdArgs.size() < 1)
        return;
    if (auto* pn = static_cast<PrimaryNode*>(current)) {
        pn->addAxiom(cmdArgs[0]);
        printIncr(pn);
    }
}


void MathlangPlugin::viewTypes(vector<string> cmdArgs) {
    CALL_INFO("типы", "<типы> - перечислить уже определённые типы.")

    write(INFO_TYPE::TXT, "Опеределены следующие типы:");
    for (const auto& n : current->index().getNames(NameTy::MT))
        write(INFO_TYPE::TXT, n);
}

void MathlangPlugin::viewSyms(vector<string> cmdArgs) {
    CALL_INFO("символы", "<символы> - перечислить уже определённые символы.")

    write(INFO_TYPE::TXT, "Опеределены следующие имена символов:");
    for (const auto& n : current->index().getNames(NameTy::SYM))
        write(INFO_TYPE::TXT, n);
}

void MathlangPlugin::viewVars(vector<string> cmdArgs) {
    CALL_INFO("переменные", "<переменные> - перечислить уже определённые переменные.")

    write(INFO_TYPE::TXT, "Опеределены следующие имена переменных:");
    for (const auto& n : current->index().getNames(NameTy::VAR))
        write(INFO_TYPE::TXT, n);
}

void MathlangPlugin::deduceMP(vector<string> cmdArgs) {
    CALL_INFO("MP",
             "<MP [PathPre] [PathImpl]> - применить правило вывода MP для Pre и Impl.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = static_cast<PrimaryNode*>(current)) {
        pn->doMP(cmdArgs[0], cmdArgs[1]);
        printIncr(pn);
    }
}

void MathlangPlugin::deduceSpec(vector<string> cmdArgs) {
    CALL_INFO("spec",
             "<spec [PathGen] [PathT]> - применить правило вывода Spec для Gen и T.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = static_cast<PrimaryNode*>(current)) {
        pn->doSpec(cmdArgs[0], cmdArgs[1]);
        printIncr(pn);
    }
}

void MathlangPlugin::deduceGen(vector<string> cmdArgs) {
    CALL_INFO("gen",
             "<gen [PathToGen] [PathVar]> - применить правило вывода Gen для ToGen и Var.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = static_cast<PrimaryNode*>(current)) {
        pn->doGen(cmdArgs[0], cmdArgs[1]);
        printIncr(pn);
    }
}

MathlangPlugin::MathlangPlugin(BaseModule* _parent, SharedObject* _fabric)
: BaseModule(ModuleInfo("Теория типов", "0.3", "04.07.17"), _parent)
{
    // Инициализация переменных здесь
    storage = new BranchNode("Главный");
    current = storage;

    methodsCfg();
    fabric = _fabric;
}

void MathlangPlugin::methodsCfg()
{
    methods.insert(make_pair("start_course" , &MathlangPlugin::startCourse ));
    methods.insert(make_pair("start_section", &MathlangPlugin::startSection));
    methods.insert(make_pair("start_lecture", &MathlangPlugin::startLecture));

    methods.insert(make_pair("to_super",&MathlangPlugin::toSuper));
    methods.insert(make_pair("to_sub",  &MathlangPlugin::toSubNode));
    methods.insert(make_pair("view", &MathlangPlugin::viewNode));
    methods.insert(make_pair("save", &MathlangPlugin::saveAll));
    methods.insert(make_pair("load", &MathlangPlugin::loadAll));

    methods.insert(make_pair("add_type" , &MathlangPlugin::addType));
    methods.insert(make_pair("add_sym"  , &MathlangPlugin::addSym));
    methods.insert(make_pair("add_var"  , &MathlangPlugin::addVar));
    methods.insert(make_pair("add_axiom", &MathlangPlugin::addAxiom));

    methods.insert(make_pair("view_types",&MathlangPlugin::viewTypes));
    methods.insert(make_pair("view_syms", &MathlangPlugin::viewSyms));
    methods.insert(make_pair("view_vars", &MathlangPlugin::viewVars));



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
{ delete one; }
