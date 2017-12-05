//
// Created by anton on 04.07.17.
//

#include "../../core.hpp"
#include "../../lib/mathlang/rationale.hpp"
#include "../../lib/mathlang/parser.hpp"
#include "../../lib/mathlang/view.hpp"

using namespace std;

class MathlangPlugin : public BaseModule
{
public:
    typedef map<string, string> MlFileIndex;
private:
    // Здесь описываются необходимые переменные
    NamedNode* storage;
    NamedNode* current;
    void resetStorage(NamedNode* _storage);
    string userCheck() const;
    void printIncr();
    json getUserIndex(const string& userName = "");

    // Далее следуют функции, реализующие функционал плагина
    void startCourse (vector<string> cmdArgs);
    void startSection(vector<string> cmdArgs);
    void startLecture(vector<string> cmdArgs);

    void toSuper  (vector<string> cmdArgs);
    void toSubNode(vector<string> cmdArgs);
    void viewNode (vector<string> cmdArgs);

    void saveAs(vector<string> cmdArgs);
    void saveChanges(vector<string> cmdArgs);
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

void MathlangPlugin::resetStorage(NamedNode* _storage) {
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

void MathlangPlugin::printIncr() {
    AsMlObj asMlObj;
    current->Node::print(&asMlObj, true);
    for (const auto& l : asMlObj.buffer)
        write(INFO_TYPE::ML_OBJ, l);
}

void MathlangPlugin::startCourse(vector<string> cmdArgs) {
    CALL_INFO("курс", "<курс [title]> - начать курс с названием title.")

    if (auto* bn = dynamic_cast<BranchNode*>(current)) {
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

    if (auto* bn = dynamic_cast<BranchNode*>(current)) {
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

    if (auto* bn = dynamic_cast<BranchNode*>(current)) {
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

    auto* target = static_cast<NamedNode*>(current->getParent());
    if (target)
        current = target;
}

void MathlangPlugin::toSubNode(vector<string> cmdArgs) {
    CALL_INFO("перейти",
              "<перейти [номер]> - перейти в подраздел с данным порядковым номером.")

    if (cmdArgs.size() < 1)
        return;

    if (auto* bn = dynamic_cast<BranchNode*>(current)) {
        NamedNode* target = static_cast<NamedNode*>(    // у BranchNode subs гарантированно
                bn->getSub( atoi(cmdArgs[0].c_str()) ));// имеют тип NamedNode
        if (target)
            current = target;
        printIncr();
    }
}

void MathlangPlugin::viewNode(vector<string> cmdArgs) {
    CALL_INFO("показать", "<показать> - показать работу целиком.")

    AsMlObj asMlObj;
    current->Node::print(&asMlObj, false);
    for (const auto& l : asMlObj.buffer)
        write(INFO_TYPE::ML_OBJ, l);
}


string random_string(size_t length) {
    auto randchar = []() -> char {
        string charset = "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = charset.length() - 1;
        return charset[rand() % max_index];
    };
    string str(length, 0);
    generate_n(str.begin(), length, randchar);
    return str;
}

string MathlangPlugin::userCheck() const {
    BaseModule* parent = this->getParent();
    while (BaseModule* p = parent->getParent())
        parent = p;
    auto* core = static_cast<Core*>(parent); // предполагается, что в корне обязательно Core

    string userName = core->user();
    if (userName == "?")
        return "";
    return userName;
}

json MathlangPlugin::getUserIndex(const string& userName) {
    string indexFileName;
    if (!userName.empty())
        indexFileName = "data/users/" + userName + "/mathlang/index.json";
    else
        indexFileName = "data/mathlang/index.json";

    ifstream userIndexFile(indexFileName);
    if (!userIndexFile.is_open()) {
        write(INFO_TYPE::TXT, "Ошибка: не удалось получить список доступных файлов.");
        throw std::invalid_argument("Файл \"" + indexFileName + " недоступен.");
    }
    stringstream buf;
    buf << userIndexFile.rdbuf();
    return json::parse(buf.str());
}

bool isFNameCollision(const json& ind, string value) {
    for (const auto& i : ind)
        if (i.at(1) == value)
            return true;
    return false;
}

void MathlangPlugin::saveAs(vector<string> cmdArgs) {
    CALL_INFO("создать",
              "<создать [имя]> - сохранить работу в новый файл [имя].")

    string userName = userCheck();
    if (userName.empty()) {
        write(INFO_TYPE::TXT, "Ошибка: сохранение недоступно в анонимном режиме.");
        return;
    }

    json ind;
    try { ind = getUserIndex(userName); }
    catch (std::invalid_argument&) { return; }

    auto search = ind.find(cmdArgs[0]);
    if (search != ind.end()) {
        write(INFO_TYPE::TXT, "Ошибка: такое имя уже используется.");
        return;
    }
    storage->setName(cmdArgs[0]);
    string fileName = random_string(16);
    while (isFNameCollision(ind, fileName))
        fileName = random_string(16);

    ind[cmdArgs[0]] = fileName;
    {
        string indexFileName = "data/users/" + userName + "/mathlang/index.json";
        ofstream userIndexFile(indexFileName, ios::trunc);
        userIndexFile << ind.dump(2) << endl;
    }

    string filePath = "data/users/" + userName + "/mathlang/" + fileName;

    ofstream osf(filePath);
    if (!osf.is_open()) {
        write(INFO_TYPE::TXT, "Ошибка: не удалось создать файл.");
        return;
    }
    auto aj = new AsJson();
    osf << storage->print(aj, false) << endl;
    write(INFO_TYPE::TXT, "Сохранено.");
    delete aj;
}

void MathlangPlugin::saveChanges(vector<string> cmdArgs) {
    CALL_INFO("сохранить",
              "<сохранить> - запись изменения в файл.")

    string userName = userCheck();
    if (userName.empty()) {
        write(INFO_TYPE::TXT, "Ошибка: сохранение недоступно в анонимном режиме.");
        return;
    }

    json ind;
    try { ind = getUserIndex(userName); }
    catch (std::invalid_argument&) { return; }

    string fileName, filePath;
    string stTitle = storage->getName();
    if (stTitle != "Новый курс" && ind.find(stTitle) != ind.end())
            fileName = ind[storage->getName()];
    else {
        write(INFO_TYPE::TXT, "Ошибка: эта команда не для первичного сохранения.");
        return;
    }
    filePath = "data/users/" + userName + "/mathlang/" + fileName;

    ofstream osf(filePath);
    if (!osf.is_open()) {
        write(INFO_TYPE::TXT, "Ошибка: не удалось создать файл.");
        return;
    }
    auto aj = new AsJson();
    osf << storage->print(aj, false) << endl;
    write(INFO_TYPE::TXT, "Сохранено.");
    delete aj;
}

void MathlangPlugin::loadAll(vector<string> cmdArgs) {
    CALL_INFO("загрузить", "<загрузить> - открыть ранее сохраненное.")

    if (cmdArgs.size() < 1)
        return;

    json commonInd, userInd;
    try { commonInd = getUserIndex(); }
    catch (std::invalid_argument&) { return; }

    string userName = userCheck();
    if (userName.empty()) {
        try { userInd = getUserIndex(userName); }
        catch (std::invalid_argument&) { return; }
    }

    string fileName = "";
    if (userInd.find(cmdArgs[0]) != userInd.end())
        fileName = "data/users/" + userName + "/mathlang/" + userInd[cmdArgs[0]];
    else if (commonInd.find(cmdArgs[0]) != commonInd.end())
        fileName = "data/users/" + userName + "/mathlang/" + commonInd[cmdArgs[0]];

    ifstream isf(fileName);
    if (!isf.is_open()) {
        write(INFO_TYPE::TXT, "Ошибка: не удалось открыть файл.");
        return;
    }

    stringstream buf;
    buf << isf.rdbuf();
    json j = json::parse(buf.str());
    // так как объекты всегда хранятся в массиве, а на верхнем
    // уровне есть только один, берём j.at(0) а его данные - .at(1)
    BranchNode* read = BranchNode::fromJson(j.at(0).at(1));
    if (read)
        resetStorage(read);
    printIncr();
}

void MathlangPlugin::addType(vector<string> cmdArgs) {
    CALL_INFO("тип", "<тип [typeName]> - ввести тип typeName.")

    if (cmdArgs.size() < 1)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try {
            pn->defType(cmdArgs[0]);
            printIncr();
        }
        catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
    }
}

void MathlangPlugin::addSym(vector<string> cmdArgs) {
    CALL_INFO("символ",
             "<символ [symName] [argT]* [retT]> - ввести символ symName.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        list<string> argTypes(next(cmdArgs.begin()), prev(cmdArgs.end()));
        try {
            pn->defSym(cmdArgs.front(), argTypes, cmdArgs.back());
            printIncr();
        }
        catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
    }
}

void MathlangPlugin::addVar(vector<string> cmdArgs) {
    CALL_INFO("переменная",
             "<переменная [varName] [varType]> - ввести переменную varName типа varType.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try {
            pn->defVar(cmdArgs[0], getType(pn->index(), cmdArgs[1]).getName());
            printIncr();
        }
        catch (std::exception& e) { write(INFO_TYPE::TXT, string("Ошибка: ") + e.what()); }
    }
}

void MathlangPlugin::addAxiom(vector<string> cmdArgs) {
    CALL_INFO("пусть", "<пусть [statement]> - ввести утверждение statement.")

    if (cmdArgs.size() < 1)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        pn->addAxiom(cmdArgs[0]);
        printIncr();
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
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        pn->doMP(cmdArgs[0], cmdArgs[1]);
        printIncr();
    }
}

void MathlangPlugin::deduceSpec(vector<string> cmdArgs) {
    CALL_INFO("spec",
             "<spec [PathGen] [PathT]> - применить правило вывода Spec для Gen и T.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        pn->doSpec(cmdArgs[0], cmdArgs[1]);
        printIncr();
    }
}

void MathlangPlugin::deduceGen(vector<string> cmdArgs) {
    CALL_INFO("gen",
             "<gen [PathToGen] [PathVar]> - применить правило вывода Gen для ToGen и Var.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        pn->doGen(cmdArgs[0], cmdArgs[1]);
        printIncr();
    }
}

MathlangPlugin::MathlangPlugin(BaseModule* _parent, SharedObject* _fabric)
: BaseModule(ModuleInfo("Теория типов", "0.3", "04.07.17"), _parent) {
    // Инициализация переменных здесь
    storage = new BranchNode("Новый курс");
    current = storage;

    methodsCfg();
    fabric = _fabric;
}

void MathlangPlugin::methodsCfg() {
    methods.insert(make_pair("start_course" , &MathlangPlugin::startCourse ));
    methods.insert(make_pair("start_section", &MathlangPlugin::startSection));
    methods.insert(make_pair("start_lecture", &MathlangPlugin::startLecture));

    methods.insert(make_pair("to_super",&MathlangPlugin::toSuper));
    methods.insert(make_pair("to_sub",  &MathlangPlugin::toSubNode));
    methods.insert(make_pair("view", &MathlangPlugin::viewNode));

    methods.insert(make_pair("save_as", &MathlangPlugin::saveAs));
    methods.insert(make_pair("save_changes", &MathlangPlugin::saveChanges));
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

void MathlangPlugin::ifaceCfg() {
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

void MathlangPlugin::write(const INFO_TYPE& type, const std::string& mess) {
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
