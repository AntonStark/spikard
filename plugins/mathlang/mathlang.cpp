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
    Reasoning reasoning;

    // Далее следуют функции, реализующие функционал плагина
    void addType(vector<string> cmdArgs);
    void addSym(vector<string> cmdArgs);
    void addVar(vector<string> cmdArgs);
    void addStatement(vector<string> cmdArgs);

    void viewTypes(vector<string> cmdArgs);
    void viewSyms(vector<string> cmdArgs);
    void viewVars(vector<string> cmdArgs);
    void viewReas(vector<string> cmdArgs);

    void saveReas(vector<string> cmdArgs);
    void loadReas(vector<string> cmdArgs);

    void deduceMP(vector<string> cmdArgs);
    void deduceSpec(vector<string> cmdArgs);

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
        reasoning.addType(cmdArgs[0]);
        cout << "Объявлен тип " << cmdArgs[0] << "." << endl;
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
    list<MathType> argTypes;
    for (auto it = next(cmdArgs.begin()); it != prev(cmdArgs.end()); ++it)
        argTypes.push_back(reasoning.getT(*it));
    try {
        reasoning.addSym(cmdArgs.front(), argTypes, cmdArgs.back());

        cout << "Введен символ " << cmdArgs[0] << " : ";
        if (argTypes.size() > 0)
        {
            cout << argTypes.front().getName();
            auto e = argTypes.end();
            for (auto it = next(argTypes.begin()); it != e; ++it)
                cout << " x " << it->getName();
        }
        cout << " -> " << cmdArgs.back() << "." << endl;
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
        reasoning.addVar(cmdArgs[0], reasoning.getT(cmdArgs[1]));
        cout << "Добавлена переменная " << cmdArgs[0] << " типа " << cmdArgs[1] << "." << endl;
    }
    catch (std::exception& e) { cout << "Ошибка: " << e.what() << endl; }
}

void MathlangPlugin::addStatement(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "пусть",
             "<пусть [statement]> - ввести утверждение statement."))
        return;

    if (cmdArgs.size() < 1)
        return;
    ::addStatement(reasoning, cmdArgs[0]);
}


void MathlangPlugin::viewTypes(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "типы",
             "<типы> - перечислить уже определённые типы."))
        return;

    set<string> buf;
    reasoning.viewSetOfNames(buf, Namespace::NameTy::MT);
    for (auto& b : buf)
        cout << b << endl;
}

void MathlangPlugin::viewSyms(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "символы",
             "<символы> - перечислить уже определённые символы."))
        return;

    set<string> buf;
    reasoning.viewSetOfNames(buf, Namespace::NameTy::SYM);
    for (auto& b : buf)
        cout << b << endl;
}

void MathlangPlugin::viewVars(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "переменные",
             "<переменные> - перечислить уже определённые переменные."))
        return;

    set<string> buf;
    reasoning.viewSetOfNames(buf, Namespace::NameTy::VAR);
    for (auto& b : buf)
        cout << b << endl;
}

void MathlangPlugin::viewReas(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "показать_рассуждение",
                 "<показать_рассуждение> - показать рассуждение целиком."))
        return;
    cout << reasoning << endl;
}

void MathlangPlugin::saveReas(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "сохранить_рассуждение",
                 "<сохранить_рассуждение> - запись для последующего использования."))
        return;
}

void MathlangPlugin::loadReas(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "загрузить_рассуждение",
                 "<загрузить_рассуждение> - открыть ранее сохраненное."))
        return;
}

Path mkPath(std::string source)
{
    // string  ->  list<size_t>
    // (1.2.2) }-> {1,2,2}
    Path target;
    map<char, unsigned> digits = {{'0', 0}, {'1', 1},
                              {'2', 2}, {'3', 3}, {'4', 4}, {'5', 5},
                              {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}};
    unsigned buf = 0;
    for (int i = 1; i < source.length()-1; ++i)
    {
        auto search = digits.find(source[i]);
        if (search != digits.end())
        {
            buf *= 10;
            buf += search->second;
        }
        else if (source[i] == '.')
        {
            target.push_back(buf);
            buf = 0;
        }
        else
            return {};
    }
    target.push_back(buf);
    return target;
}

void MathlangPlugin::deduceMP(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "MP",
             "<MP [PathPre] [PathImpl]> - применить правило вывода MP для Pre и Impl."))
        return;

    if (cmdArgs.size() < 2)
        return;
    Path pathPre = mkPath(cmdArgs[0]);
    Path pathImpl = mkPath(cmdArgs[1]);
    reasoning.deduceMP(pathPre, pathImpl);
}

void MathlangPlugin::deduceSpec(vector<string> cmdArgs)
{
    if (funcInfo(cmdArgs, "spec",
             "<spec [PathGen] [PathT]> - применить правило вывода Spec для Gen и T."))
        return;

    if (cmdArgs.size() < 2)
        return;
    Path pathGen = mkPath(cmdArgs[0]);
    Path pathT = mkPath(cmdArgs[1]);
    reasoning.deduceSpec(pathGen, pathT);
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
    methods.insert(make_pair("add_type" , &MathlangPlugin::addType));
    methods.insert(make_pair("add_sym"  , &MathlangPlugin::addSym));
    methods.insert(make_pair("add_var"  , &MathlangPlugin::addVar));
    methods.insert(make_pair("add_statement", &MathlangPlugin::addStatement));

    methods.insert(make_pair("view_types", &MathlangPlugin::viewTypes));
    methods.insert(make_pair("view_syms", &MathlangPlugin::viewSyms));
    methods.insert(make_pair("view_vars", &MathlangPlugin::viewVars));
    methods.insert(make_pair("view_reas", &MathlangPlugin::viewReas));

    methods.insert(make_pair("save_reas", &MathlangPlugin::saveReas));
    methods.insert(make_pair("load_reas", &MathlangPlugin::loadReas));

    methods.insert(make_pair("deduce_MP", &MathlangPlugin::deduceMP));
    methods.insert(make_pair("deduce_Spec", &MathlangPlugin::deduceSpec));
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