//
// Created by anton on 25.11.17.
//

#include "view.hpp"

std::string PlainText::process(const ListStorage* ls) {
    std::stringstream buf;
    auto e = ls->end();
    indent+=2;
    for (auto it = ls->start(); it != e; ++it)
        buf << printInd() << "(" << (*it)->getNumber() << "): " <<
                (*it)->print(this) << std::endl;
    indent-=2;
    return buf.str();
}

std::string PlainText::process(const NamedNode* nn)
{
    switch (nn->_type) {
        case NamedNodeType::COURSE  :
            return ("Курс \"" + nn->getName() + "\".");
        case NamedNodeType::SECTION : {
            std::stringstream ss;
            ss << "Раздел " << nn->getNumber() <<
            ": " << nn->getName() << ".";
            return ss.str();
        }
        case NamedNodeType::LECTURE :
            return ("Лекция \"" + nn->getName() + "\".");
        case NamedNodeType::CLOSURE :
            return "";
    }
}
std::string PlainText::process(const DefType* dt)
{ return ("Объявлен тип " + dt->getName() + "."); }

std::string PlainText::process(const DefVar* dv)
{ return ("Добавлена переменная " + dv->getName() +
          " типа " + dv->getType().getName() + "."); }

std::string PlainText::process(const DefSym* ds) {
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
    return buf.str();
}

std::string PlainText::process(const Axiom* ax) {
    std::stringstream buf;
    buf << "Пусть ";
    ax->get()->print(buf);
    return buf.str();
}

std::string PlainText::process(const AbstrInf* ai) {
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
    return buf.str();
}

std::string AsJson::process(const ListStorage* ls) {

}

std::string AsJson::process(const NamedNode* nn) {
    json temp;
    temp["ItemType"] = "NamedNode";
    temp["ItemData"] = { {"storing_straegy", nn->nssType()},
                         {"type", toStr(nn->_type)},
                         {"title", nn->getName()} };
    return temp.dump(2);
}

std::string AsJson::process(const DefType* dt) {
    json temp;
    temp["ItemType"] = "DefType";
    temp["ItemData"] = { {"name", dt->getName()} };
    return temp.dump(2);
}

std::string AsJson::process(const DefVar* dv) {
    json temp;
    temp["ItemType"] = "DefVar";
    temp["ItemData"] = { {"name", dv->getName()},
                         {"type", dv->getType().getName()} };
    return temp.dump(2);
}

std::string AsJson::process(const DefSym* ds) {
    json temp;
    auto symInfo = ds->getSign();
    std::vector<std::string> argT;
    for (const auto& a : symInfo.first)
        argT.push_back(a.getName());

    temp["ItemType"] = "DefSym";
    temp["ItemData"] = { {"name", ds->getName()},
                         {"argT", argT},
                         {"retT", symInfo.second.getName()}};
    return temp.dump(2);
}

std::string AsJson::process(const Axiom* ax) {
    json temp;
    std::stringstream ss;
    ax->get()->print(ss);
    temp["ItemType"] = "Axiom";
    temp["ItemData"] = { {"axiom", ss.str()} };
    return temp.dump(2);
}

std::string AsJson::process(const AbstrInf* ai) {
    json temp;
    switch (ai->type)
    {
        case AbstrInf::InfTy::MP : { temp["ItemType"] = "InfMP"; break; }
        case AbstrInf::InfTy::GEN : { temp["ItemType"] = "InfGen"; break; }
        case AbstrInf::InfTy::SPEC : { temp["ItemType"] = "InfSpec"; break; }
    }
    temp["ItemData"] = { {"arg1", pathToStr(ai->premises.at(0))},
                         {"arg2", pathToStr(ai->premises.at(1))} };
    return temp.dump(2);
}
