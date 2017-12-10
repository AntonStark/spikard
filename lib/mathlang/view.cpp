//
// Created by anton on 25.11.17.
//

#include "view.hpp"

void PlainText::process(const ListStorage* ls) {
    std::stringstream buf;
    auto e = ls->end();
    indent+=2;
    for (auto it = ls->start(); it != e; ++it)
        buf << printInd() << "(" << (*it)->getNumber() << "): " <<
                (*it)->print(this) << std::endl;
    indent-=2;
    value = buf.str();
}

void PlainText::process(const NamedNode* nn)
{
    switch (nn->_type) {
        case NamedNodeType::COURSE  :
            value = ("Курс \"" + nn->getName() + "\"."); break;
        case NamedNodeType::SECTION : {
            std::stringstream ss;
            ss << "Раздел " << nn->getNumber() <<
            ": " << nn->getName() << ".";
            value = ss.str(); break;
        }
        case NamedNodeType::LECTURE :
            value = ("Лекция \"" + nn->getName() + "\"."); break;
        case NamedNodeType::CLOSURE :
            value = ""; break;
    }
}
void PlainText::process(const DefType* dt)
{ value = ("Объявлен тип " + dt->getName() + "."); }

void PlainText::process(const DefVar* dv)
{ value = ("Добавлена переменная " + dv->getName() +
          " типа " + dv->getType().getName() + "."); }

void PlainText::process(const DefSym* ds) {
    std::stringstream buf;
    buf << "Введен символ " << ds->getName() << " : ";
    auto argTypes = ds->getSign().first;
    if (!argTypes.empty())
    {
        buf << argTypes.front().getName();
        auto e = argTypes.end();
        for (auto it = std::next(argTypes.begin()); it != e; ++it)
            buf << " x " << it->getName();
    }
    buf << " -> " << ds->getType().getName() << ".";
    value = buf.str();
}

void PlainText::process(const Axiom* ax) {
    std::stringstream buf;
    buf << "Пусть ";
    ax->get()->print(buf);
    value = buf.str();
}

void PlainText::process(const AbstrInf* ai) {
    std::stringstream buf;
    buf << "По ";
    switch (ai->type)
    {
        case AbstrInf::InfTy::MP :
            buf << "MP ";   break;
        case AbstrInf::InfTy::SPEC :
            buf << "Spec "; break;
        case AbstrInf::InfTy::GEN :
            buf << "Gen ";  break;
    }
    buf << "из " <<
            pathToStr(ai->premises[0]) << " и " <<
            pathToStr(ai->premises[1]) << " следует: ";
    ai->get()->print(buf);
    value = buf.str();
}


void AsJson::process(const ListStorage* ls) {
    auto e = ls->end();
    for (auto it = ls->start(); it != e; ++it)
        (*it)->print(this, false);
}

void AsJson::process(const NamedNode* nn) {
    auto* listRep = new AsJson();
    nn->ListStorage::print(listRep, false);  // json для потомков перехватываем в listRep
    value.push_back({"NamedNode",
                    {
                        {"storing_strategy", nn->nssType()},
                        {"type", toStr(nn->_type)},
                        {"title", nn->getName()},
                        {"subs", listRep->value}
                    } });
}

void AsJson::process(const DefType* dt) {
    value.push_back({"DefType",
                    {{"name", dt->getName()}}
                   });
}

void AsJson::process(const DefVar* dv) {
    value.push_back({"DefVar",
                    {
                        {"name", dv->getName()},
                        {"type", dv->getType().getName()}
                    } });
}

void AsJson::process(const DefSym* ds) {
    auto symInfo = ds->getSign();
    std::vector<std::string> argT;
    for (const auto& a : symInfo.first)
        argT.push_back(a.getName());

    value.push_back({"DefSym",
                    {
                        {"name", ds->getName()},
                        {"argT", argT},
                        {"retT", symInfo.second.getName()}
                    } });
}

void AsJson::process(const Axiom* ax) {
    std::stringstream ss;
    ax->get()->print(ss);
    value.push_back({"Axiom",
                    {{"axiom", ss.str()}}
                   });
}

void AsJson::process(const AbstrInf* ai) {
    std::string infType;
    switch (ai->type)
    {
        case AbstrInf::InfTy::MP  : { infType = "InfMP";   break; }
        case AbstrInf::InfTy::GEN : { infType = "InfGen";  break; }
        case AbstrInf::InfTy::SPEC: { infType = "InfSpec"; break; }
    }
    value.push_back({infType,
                    {
                        {"arg1", pathToStr(ai->premises.at(0))},
                        {"arg2", pathToStr(ai->premises.at(1))}
                    } });
}


void AsMlObj::process(const ListStorage* ls) {
    auto e = ls->end();
    for (auto it = ls->start(); it != e; ++it)
        (*it)->print(this, false);
}

void AsMlObj::process(const NamedNode* nn) {
    std::stringstream body;
    body << nn->getNumber() << ". " << toStr(nn->_type)
         << " \"" << nn->getName() << "\".";
    buffer.push_back(
        MlObj("named_node", nn->getNumber(), body.str())
        .toJson().dump(2)
    );
}

void AsMlObj::process(const DefType* dt) {
    buffer.push_back(
        MlObj("def_type", dt->getNumber(), dt->getName())
            .toJson().dump(2)
    );
}

void AsMlObj::process(const DefVar* dv) {
    std::stringstream body;
    body << dv->getName() << "\\in " << dv->getType().getName();
    buffer.push_back(
        MlObj("def_var", dv->getNumber(), body.str())
            .toJson().dump(2)
    );
}

void AsMlObj::process(const DefSym* ds) {
    std::stringstream out;
    out << ds->getName() << " : ";
    auto argTypes = ds->getSign().first;
    if (!argTypes.empty())
    {
        out << argTypes.front().getName();
        auto e = argTypes.end();
        for (auto it = next(argTypes.begin()); it != e; ++it)
            out << "\\times " << it->getName();
    }
    out << "\\rightarrow " << ds->getType().getName();
    buffer.push_back(
        MlObj("def_sym", ds->getNumber(), out.str())
            .toJson().dump(2)
    );
}

void AsMlObj::process(const Axiom* ax) {
    std::stringstream formula;
    ax->get()->print(formula);
    buffer.push_back(
        MlObj("axiom", ax->getNumber(), formula.str())
            .toJson().dump(2)
    );
}

void AsMlObj::process(const AbstrInf* ai) {
    std::string infType;
    switch (ai->type)
    {
        case AbstrInf::InfTy::MP   : { infType = "inf_mp";  break; }
        case AbstrInf::InfTy::GEN  : { infType = "inf_gen"; break; }
        case AbstrInf::InfTy::SPEC : { infType = "inf_spec";break; }
    }
    std::stringstream inf;
    ai->get()->print(inf);
    buffer.push_back(
        MlObj(infType, ai->getNumber(), inf.str(), ai->premises)
            .toJson().dump(2)
    );
}