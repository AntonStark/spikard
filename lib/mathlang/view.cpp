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
          " типа " + dv->getType()->getName() + "."); }

void PlainText::process(const DefSym* ds) {
    std::stringstream buf;
    buf << "Введен символ " << ds->getName() << " : ";
    auto argTypes = ds->getArgs()->getNames();
    if (!argTypes.empty())
    {
        buf << argTypes.front();
        auto e = argTypes.end();
        for (auto it = std::next(argTypes.begin()); it != e; ++it)
            buf << " x " << *it;
    }
    buf << " -> " << ds->getType()->getName() << ".";
    value = buf.str();
}

void PlainText::process(const TermsBox* ax) {
    std::stringstream buf;
    buf << "Пусть " << ax->get()->print();
    value = buf.str();
}

void PlainText::process(const Inference* inf) {
    std::stringstream buf;
    switch (inf->type) {
        case Inference::InfTy::MP : {
            buf << "По ModusPonens из " <<
                pathToStr(inf->premises[0]) << " и " <<
                pathToStr(inf->premises[1]) << " следует: ";
            break;
        }
        case Inference::InfTy::SPEC :
        case Inference::InfTy::GEN  : {
            buf << "По " <<
                pathToStr(inf->premises[0]) << " для " <<
                pathToStr(inf->premises[1]) << " получаем: ";
            break;
        }
    }
    buf << inf->get()->print();
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
                        {"type", dv->getType()->getName()}
                    } });
}

void AsJson::process(const DefSym* ds) {
    value.push_back({"DefSym",
                    {
                        {"name", ds->getName()},
                        {"argT", ds->getArgs()->getNames()},
                        {"retT", ds->getType()->getName()}
                    } });
}

void AsJson::process(const TermsBox* ax) {
    value.push_back({"TermsBox",
                    {{"axiom", ax->get()->print()}}
                   });
}

void AsJson::process(const Inference* inf) {
    value.push_back({"Inference",
                     {
                         {"premise1", pathToStr(inf->premises.at(0))},
                         {"premise2", pathToStr(inf->premises.at(1))},
                         {"type", inf->getTypeAsStr()}
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
        MlObj("named_node", nn->getNumber(), body.str()).toJson()
    );
}

void AsMlObj::process(const DefType* dt) {
    buffer.push_back(
        MlObj("def_type", dt->getNumber(), dt->getName()).toJson()
    );
}

void AsMlObj::process(const DefVar* dv) {
    std::stringstream body;
    body << dv->getName() << "\\in " << dv->getType()->getName();
    buffer.push_back(
        MlObj("def_var", dv->getNumber(), body.str()).toJson()
    );
}

void AsMlObj::process(const DefSym* ds) {
    std::stringstream out;
    out << ds->getName() << " : ";
    auto argTypes = ds->getArgs()->getNames();
    if (!argTypes.empty())
    {
        out << argTypes.front();
        auto e = argTypes.end();
        for (auto it = next(argTypes.begin()); it != e; ++it)
            out << "\\times " << *it;
    }
    out << "\\rightarrow " << ds->getType()->getName();
    buffer.push_back(
        MlObj("def_sym", ds->getNumber(), out.str()).toJson()
    );
}

void AsMlObj::process(const TermsBox* ax) {
    buffer.push_back(
        MlObj("term", ax->getNumber(), ax->get()->print()).toJson()
    );
}

void AsMlObj::process(const Inference* inf) {
    buffer.push_back(MlObj("inference",
                           inf->getNumber(),
                           inf->get()->print(),
                           inf->premises)
                         .toJson());
    buffer.back()["type"] = inf->getTypeAsStr();
}
