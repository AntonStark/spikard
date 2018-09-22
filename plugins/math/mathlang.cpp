//
// Created by anton on 04.07.17.
//

#include "../../core.hpp"
#include "../../lib/mathlang/proof/definition.hpp"
#include "../../lib/mathlang/parser/old_parser.hpp"
#include "../../lib/mathlang/view.hpp"

using namespace std;

class MathlangPlugin : public BaseModule
{
private:
    // Здесь описываются необходимые переменные
    NamedNode* storage;
    NamedNode* current;
    void resetStorage(NamedNode* _storage);
    void tellTypeNames();
    string userCheck() const;
    void print(bool incr);
    json getIndexFromFile(const string& indexFilePath);

    // Далее следуют функции, реализующие функционал плагина
    void startCourse (vector<string> cmdArgs);
    void startSection(vector<string> cmdArgs);
    void startLecture(vector<string> cmdArgs);

    void toPar(vector<string> cmdArgs);
    void toSubNode(vector<string> cmdArgs);
    void viewWork(vector<string> cmdArgs);

    void saveAs(vector<string> cmdArgs);
    void saveChanges(vector<string> cmdArgs);
    void loadAll(vector<string> cmdArgs);
    void viewIndex(vector<string> cmdArgs);

    void addType(vector<string> cmdArgs);
    void addSym(vector<string> cmdArgs);
    void addVar(vector<string> cmdArgs);
    void addTerm(vector<string> cmdArgs);

    void viewTypes(vector<string> cmdArgs);
    void viewSyms(vector<string> cmdArgs);
    void viewVars(vector<string> cmdArgs);

    void deduceMP  (vector<string> cmdArgs);
    void deduceSpec(vector<string> cmdArgs);
    void deduceApply(vector<string> cmdArgs);
    void deduceEqual(vector<string> cmdArgs);
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
    void write(const INFO_TYPE&, const json& mess) override;
};

void MathlangPlugin::resetStorage(NamedNode* _storage) {
    delete storage;
    storage = _storage;
    current = storage;
}

void MathlangPlugin::tellTypeNames() {
    json types = { {"types", current->index().getNames(NameTy::MT)} };
    write(INFO_TYPE::NAME, types);
}

void MathlangPlugin::print(bool incr = true) {
    AsMlObj asMlObj;
    if (!incr) {
        current->print(&asMlObj);
        asMlObj.buffer.front()["label"] = {0};
        auto primary = dynamic_cast<PrimaryNode*>(current);
        asMlObj.buffer.front()["is_primary_node"] = (primary != nullptr);
    }
    current->Node::print(&asMlObj, incr);
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
        print();
    }
    else
        write(INFO_TYPE::ERR,
              "нельзя начать курс в первичном узле, перейдите выше.");
}

void MathlangPlugin::startSection(vector<string> cmdArgs) {
    CALL_INFO("раздел", "<раздел [title]> - начать раздел с названием title.")

    if (auto* bn = dynamic_cast<BranchNode*>(current)) {
        if (cmdArgs.size() < 1)
            bn->startSection();
        else
            bn->startSection(cmdArgs[0]);
        write(INFO_TYPE::TXT, "Раздел создан.");
        print();
    }
    else
        write(INFO_TYPE::ERR,
              "нельзя начать раздел в первичном узле, перейдите выше.");
}

void MathlangPlugin::startLecture(vector<string> cmdArgs) {
    CALL_INFO("лекция", "<лекция [title]> - начать лекцию с названием title.")

    if (auto* bn = dynamic_cast<BranchNode*>(current)) {
        if (cmdArgs.size() < 1)
            bn->startLecture();
        else
            bn->startLecture(cmdArgs[0]);
        write(INFO_TYPE::TXT, "Лекция создана.");
        print();
    }
    else
        write(INFO_TYPE::ERR,
              "нельзя начать лекцию в первичном узле, перейдите выше.");
}

void MathlangPlugin::toPar(vector<string> cmdArgs) {
    CALL_INFO("наверх", "<наверх> - перейти к узлу уровнем выше.")

    auto* target = static_cast<NamedNode*>(current->getParent());
    if (target) {
        current = target;
        write(INFO_TYPE::TXT, "Переход вверх выполнен.");
        print(false);
    }
    else
        write(INFO_TYPE::ERR, "это самый верхний узел.");
}

void MathlangPlugin::toSubNode(vector<string> cmdArgs) {
    CALL_INFO("перейти",
              "<перейти [номер]> - перейти в подраздел с данным порядковым номером.")

    if (cmdArgs.size() < 1)
        return;

    if (auto* bn = dynamic_cast<BranchNode*>(current)) {
        size_t sub = atoi(cmdArgs[0].c_str());
        if (sub < 1) {
            write(INFO_TYPE::ERR, "нумерация должна быть с единицы.");
            return;
        }
        // у BranchNode subs гарантированно имеют тип NamedNode
        NamedNode* target = static_cast<NamedNode*>(bn->getSub(sub));
        if (target) {
            current = target;
            write(INFO_TYPE::TXT, "Переход выполнен.");
            print(false);
        }
        else
            write(INFO_TYPE::ERR, "переход не удался.");
    }
}

void MathlangPlugin::viewWork(vector<string> cmdArgs) {
    CALL_INFO("показать", "<показать> - показать работу целиком.")

    print(false);
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

json MathlangPlugin::getIndexFromFile(const string& indexFilePath) {
    ifstream userIndexFile(indexFilePath);
    if (!userIndexFile.is_open()) {
        write(INFO_TYPE::ERR, "не удалось получить список доступных файлов.");
        throw std::invalid_argument("Файл \"" + indexFilePath + " недоступен.");
    }
    return json::parse(userIndexFile);
}

bool isFNameCollision(const json& ind, string value) {
    for (const auto& i : ind)
        if (i == value)
            return true;
    return false;
}

void MathlangPlugin::saveAs(vector<string> cmdArgs) {
    CALL_INFO("создать",
              "<создать [имя]> - сохранить работу в новый файл [имя].")

    string userName = userCheck();
    if (userName.empty()) {
        write(INFO_TYPE::ERR, "сохранение недоступно в анонимном режиме.");
        return;
    }

    json ind;
    string indexFileName = "data/users/" + userName + "/math/index.json";
    try { ind = getIndexFromFile(indexFileName); }
    catch (std::invalid_argument&) { return; }

    auto search = ind.find(cmdArgs[0]);
    if (search != ind.end()) {
        write(INFO_TYPE::ERR, "такое имя уже используется.");
        return;
    }
    storage->setName(cmdArgs[0]);
    string fileName = random_string(16) + ".json";
    while (isFNameCollision(ind, fileName))
        fileName = random_string(16) + ".json";

    ind[cmdArgs[0]] = fileName;
    {
        ofstream userIndexFile(indexFileName, ios::trunc);
        userIndexFile << ind.dump(2) << endl;
    }

    string filePath = "data/users/" + userName + "/math/" + fileName;

    ofstream osf(filePath);
    if (!osf.is_open()) {
        write(INFO_TYPE::ERR, "не удалось создать файл.");
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
        write(INFO_TYPE::ERR, "сохранение недоступно в анонимном режиме.");
        return;
    }

    json ind;
    string indexFileName = "data/users/" + userName + "/math/index.json";
    try { ind = getIndexFromFile(indexFileName); }
    catch (std::invalid_argument&) { return; }

    string fileName, filePath;
    string stTitle = storage->getName();
    if (stTitle != "Новый курс" && ind.find(stTitle) != ind.end())
            fileName = ind[storage->getName()];
    else {
        write(INFO_TYPE::ERR, "эта команда не для первичного сохранения.");
        return;
    }
    filePath = "data/users/" + userName + "/math/" + fileName;

    ofstream osf(filePath, ios::trunc);
    if (!osf.is_open()) {
        write(INFO_TYPE::ERR, "не удалось создать файл.");
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
    string commonIndPath = "data/math/index.json";
    try { commonInd = getIndexFromFile(commonIndPath); }
    catch (std::invalid_argument&) { return; }

    string userName = userCheck();
    if (!userName.empty()) {
        string userIndPath = "data/users/" + userName + "/math/index.json";
        try { userInd = getIndexFromFile(userIndPath); }
        catch (std::invalid_argument&) { userInd = {}; }
    }

    string fileName = "data/users/";
    if (userInd.find(cmdArgs[0]) != userInd.end()) {
        fileName += userName + "/math/" + userInd.at(cmdArgs[0]).get<string>();
    }
    else if (commonInd.find(cmdArgs[0]) != commonInd.end()) {
        auto fileInfo = commonInd.at(cmdArgs[0]);
        fileName += fileInfo.at(0).get<string>() + "/math/" + fileInfo.at(1).get<string>();
    }
    else {
        write(INFO_TYPE::ERR, "не удалось найти файл.");
        return;
    }

    ifstream isf(fileName);
    if (isf.fail()) {
        write(INFO_TYPE::ERR, "не удалось открыть файл.");
        return;
    }
    json j = json::parse(isf);
    // так как объекты всегда хранятся в массиве, а на верхнем
    // уровне есть только один, берём j.at(0) а его данные - .at(1)
    BranchNode* read = BranchNode::fromJson(j.at(0).at(1));
    if (read)
        resetStorage(read);
    print(false);
    write(INFO_TYPE::TXT, "Загружено.");
}

void MathlangPlugin::viewIndex(vector<string> cmdArgs) {
    CALL_INFO("работы", "<работы> - показать доступные для загрузки работы.")

    json commonInd, userInd;
    string commonIndPath = "data/math/index.json";
    try { commonInd = getIndexFromFile(commonIndPath); }
    catch (std::invalid_argument&) { return; }

    string userName = userCheck();
    if (!userName.empty()) {
        string userIndPath = "data/users/" + userName + "/math/index.json";
        try { userInd = getIndexFromFile(userIndPath); }
        catch (std::invalid_argument&) { userInd = {}; }
        auto e = userInd.end();
        for (auto it = userInd.begin(); it != e; ++it)
            commonInd[it.key()] = it.value();
    }
    auto e = commonInd.end();
    for (auto it = commonInd.begin(); it != e; ++it)
        write(INFO_TYPE::TXT, it.key());
}


void MathlangPlugin::addType(vector<string> cmdArgs) {
    CALL_INFO("тип", "<тип [typeName]> - ввести тип typeName.")

    if (cmdArgs.size() < 1)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try {
            pn->defType(cmdArgs[0]);
            print();
        }
        catch (std::exception& e) { write(INFO_TYPE::ERR, e.what()); return; }
    }
    else
        write(INFO_TYPE::ERR, "возможно только в первичном узле.");
}

void MathlangPlugin::addSym(vector<string> cmdArgs) {
    CALL_INFO("символ",
             "<символ [symName] [argT]* [retT]> - ввести символ symName.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        vector<string> argTypes(next(cmdArgs.begin()), prev(cmdArgs.end()));
        try {
            pn->defSym(cmdArgs.front(), argTypes, cmdArgs.back());
            print();
        }
        catch (std::exception& e) { write(INFO_TYPE::ERR, e.what()); }
    }
    else
        write(INFO_TYPE::ERR, "возможно только в первичном узле.");
}

void MathlangPlugin::addVar(vector<string> cmdArgs) {
    CALL_INFO("переменная",
             "<переменная [varName] [varType]> - ввести переменную varName типа varType.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try {
            pn->defVar(cmdArgs[0], getType(pn->index(), cmdArgs[1])->getName());
            print();
        }
        catch (std::exception& e) { write(INFO_TYPE::ERR, e.what()); }
    }
    else
        write(INFO_TYPE::ERR, "возможно только в первичном узле.");
}

void MathlangPlugin::addTerm(vector<string> cmdArgs) {
    CALL_INFO("пусть", "<пусть [statement]> - ввести утверждение statement.")

    if (cmdArgs.size() < 1)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try { pn->addTerm(cmdArgs[0]); }
        catch (std::invalid_argument& e) {
            write(INFO_TYPE::ERR, e.what());
            return;
        }
        print();
    }
    else
        write(INFO_TYPE::ERR, "возможно только в первичном узле.");
}


void MathlangPlugin::viewTypes(vector<string> cmdArgs) {
    CALL_INFO("типы", "<типы> - перечислить уже определённые типы.")

    tellTypeNames();
    if (cmdArgs.size() > 0 && cmdArgs[0] == "0")
        return;

    write(INFO_TYPE::TXT, "Определены следующие типы:");
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
        try { pn->doMP(cmdArgs[0], cmdArgs[1]); }
        catch (std::invalid_argument& e) {
            write(INFO_TYPE::ERR, e.what());
            return;
        }
        print();
    }
}

void MathlangPlugin::deduceSpec(vector<string> cmdArgs) {
    CALL_INFO("spec",
             "<spec [PathGen] [PathT]> - применить правило вывода Spec для Gen и T.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try { pn->doSpec(cmdArgs[0], cmdArgs[1]); }
        catch (std::invalid_argument& e) {
            write(INFO_TYPE::ERR, e.what());
            return;
        }
        print();
    }
}

void MathlangPlugin::deduceApply(vector<string> cmdArgs) {
    CALL_INFO("apply",
              "<apply [PathTerm] [PathTheorem]> - применить правило вывода Apply для Term и Theorem.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try { pn->doApply(cmdArgs[0], cmdArgs[1]); }
        catch (std::invalid_argument& e) {
            write(INFO_TYPE::ERR, e.what());
            return;
        }
        print();
    }
}

void MathlangPlugin::deduceEqual(vector<string> cmdArgs) {
    CALL_INFO("equal",
              "<equal [PathTerm] [PathEquality]> - применить правило вывода Equal для Term и Equality.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try { pn->doEqual(cmdArgs[0], cmdArgs[1]); }
        catch (std::invalid_argument& e) {
            write(INFO_TYPE::ERR, e.what());
            return;
        }
        print();
    }
}

void MathlangPlugin::deduceGen(vector<string> cmdArgs) {
    CALL_INFO("gen",
             "<gen [PathToGen] [PathVar]> - применить правило вывода Gen для ToGen и Var.")

    if (cmdArgs.size() < 2)
        return;
    if (auto* pn = dynamic_cast<PrimaryNode*>(current)) {
        try { pn->doGen(cmdArgs[0], cmdArgs[1]); }
        catch (std::invalid_argument& e) {
            write(INFO_TYPE::ERR, e.what());
            return;
        }
        print();
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

    methods.insert(make_pair("to_par", &MathlangPlugin::toPar));
    methods.insert(make_pair("to_sub", &MathlangPlugin::toSubNode));
    methods.insert(make_pair("view_work", &MathlangPlugin::viewWork));

    methods.insert(make_pair("load",   &MathlangPlugin::loadAll));
    methods.insert(make_pair("save_as",&MathlangPlugin::saveAs));
    methods.insert(make_pair("save_changes",&MathlangPlugin::saveChanges));
    methods.insert(make_pair("view_index",  &MathlangPlugin::viewIndex));

    methods.insert(make_pair("add_type" , &MathlangPlugin::addType));
    methods.insert(make_pair("add_sym"  , &MathlangPlugin::addSym));
    methods.insert(make_pair("add_var"  , &MathlangPlugin::addVar));
    methods.insert(make_pair("add_axiom", &MathlangPlugin::addTerm));

    methods.insert(make_pair("view_types",&MathlangPlugin::viewTypes));
    methods.insert(make_pair("view_syms", &MathlangPlugin::viewSyms));
    methods.insert(make_pair("view_vars", &MathlangPlugin::viewVars));

    methods.insert(make_pair("deduce_MP",    &MathlangPlugin::deduceMP));
    methods.insert(make_pair("deduce_Spec",  &MathlangPlugin::deduceSpec));
    methods.insert(make_pair("deduce_Apply", &MathlangPlugin::deduceApply));
    methods.insert(make_pair("deduce_Equal", &MathlangPlugin::deduceEqual));
    methods.insert(make_pair("deduce_Gen",   &MathlangPlugin::deduceGen));
}

void MathlangPlugin::ifaceCfg() {
    Core* root = static_cast<Core*>(getRoot());

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

void MathlangPlugin::write(const INFO_TYPE& type, const json& mess) {
    Core* root = static_cast<Core*>(getRoot());
    return root->write(type, mess);
}

extern "C" BaseModule* create(BaseModule* _parent, SharedObject* _fabric)
{ return new MathlangPlugin(_parent, _fabric); }

extern "C" void destroy(BaseModule* one)
{ delete one; }
