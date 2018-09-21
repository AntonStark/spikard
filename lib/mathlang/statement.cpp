//
// Created by anton on 21.09.18.
//

#include "statement.hpp"

TermsBox::TermsBox(Node* parent, std::string source)
    : Node(parent, new Hidden(parent)),
      data(parse(this, std::move(source))) {}

class Inference::bad_inf : public std::invalid_argument
{
public:
    bad_inf(const std::string& args)
        : std::invalid_argument("Неподходящие аргументы " + args +
                                " для данного вывода.") {}
};

const Terms* Inference::getTerms(Path pathToTerm) {
    Hierarchy* termItem = getParent()->getByPass(pathToTerm);
    if (auto t = dynamic_cast<Statement*>(termItem))
        return t->get();
    else if (auto v = dynamic_cast<Definition*>(termItem))
        return v->getTerm();
    else
        return nullptr;
}
const Terms* Inference::inference() {
    const Terms* arg1 = getTerms(premises[0]);
    const Terms* arg2 = getTerms(premises[1]);
    switch (type) {
        case InfTy::MP   :
            return modusPonens   (arg1, arg2);
        case InfTy::SPEC :
            return specialization(arg1, arg2);
        case InfTy::APPL :
            return application(arg1, arg2);
        case InfTy::EQL :
            return equalSubst(arg1, arg2);
        case InfTy::GEN  :
            return generalization(arg1, arg2);
    }
}
Inference::Inference(Node* naming,
                     Path pArg1, Path pArg2, Inference::InfTy _type)
    : Item(naming), premises({pArg1, pArg2}),
      type(_type), data(inference())
{ if (!data) throw bad_inf(pathToStr(pArg1) + ", " + pathToStr(pArg2)); }

std::string Inference::getTypeAsStr() const {
    switch (type)
    {
        case InfTy::MP   : return "InfMP";
        case InfTy::SPEC : return "InfSpec";
        case InfTy::APPL : return "InfAppl";
        case InfTy::EQL  : return "InfEql";
        case InfTy::GEN  : return "InfGen";
    }
}
Inference::InfTy infTyFromStr(const std::string& type) {
    typedef Inference::InfTy InfTy;
    if (type == "InfMP")
        return InfTy::MP;
    else if (type == "InfSpec")
        return InfTy::SPEC;
    else if (type == "InfAppl")
        return InfTy::APPL;
    else if (type == "InfEql")
        return InfTy::EQL;
    else
        return InfTy::GEN;
}


Terms* modusPonens(const Terms* premise, const Terms* impl) {
    Map standardImpl("\\Rightarrow ", {2, &logical_mt}, &logical_mt);
    if (const auto* tI = dynamic_cast<const Term*>(impl))
        if ((tI->getSym() == standardImpl) && tI->arg(1)->comp(premise))
            return tI->arg(2)->clone();
    return nullptr;
}

Terms* specialization(const Terms* general, const Terms* t) {
    if (const auto* fT = dynamic_cast<const ForallTerm*>(general))
        if (*fT->arg(1)->getType() == *t->getType())
            return fT->arg(2)->replace(fT->arg(1), t);
    return nullptr;
}

const Terms* innerPremise(const ForallTerm* fT) {
    const Terms* quantedterm = fT->arg(2);
    while (auto innerForall = dynamic_cast<const ForallTerm*>(quantedterm))
        quantedterm = innerForall->arg(2);
    // теперь в quantedterm лежит самый первый терм без кванторов
    if (auto notVar = dynamic_cast<const Term*>(quantedterm)) {
        Map standardImpl("\\Rightarrow ", {2, &logical_mt}, &logical_mt);
        if (notVar->getSym() == standardImpl)
            return notVar->arg(1);
    }
    return nullptr;
}
Terms::Path findVarFirstUsage(Variable var, const Term* term) {
    for (size_t i = 1; i <= term->getArity(); ++i) {
        if (auto v = dynamic_cast<const Variable*>(term->arg(i)))
        { if (var == *v) return Terms::Path({i}); }
        else {
            auto t = static_cast<const Term*>(term->arg(i));
            if (t->free.find(var) != t->free.end()) {
                Terms::Path inner = findVarFirstUsage(var, t);
                inner.push(i);
                return inner;
            }
        }
    }
    return Terms::Path();
}

const Terms* moveQuantorIntoImpl(const Term* quantedImpl) {
    const auto* topQVar = static_cast<const Variable*>(quantedImpl->arg(1));
    const Terms* topQuanted = quantedImpl->arg(2);
    Terms::Path toInnerConseq;
    const Terms* innerConseq = topQuanted;
    while (auto innerForall = dynamic_cast<const ForallTerm*>(innerConseq)) {
        innerConseq = innerForall->arg(2);
        toInnerConseq.push(2);
    }
    // quantedImpl должен содержать импликацию внутри
    innerConseq = static_cast<const Term*>(innerConseq)->arg(2);
    toInnerConseq.push(2);
    const Terms* foralledConseq = new ForallTerm(topQVar->clone(), innerConseq->clone());
    return topQuanted->replace(toInnerConseq, foralledConseq);
}
Terms* application(const Terms* term, const Terms* theorem) {
    while (auto fT = dynamic_cast<const ForallTerm*>(theorem)) {
        const Terms* foralledPremise = innerPremise(fT);
        if (!foralledPremise) // theorem должна содержать импликацию внутри
            return nullptr;
        const Variable var = *static_cast<const Variable*>(fT->arg(1));
        if (auto v = dynamic_cast<const Variable*>(foralledPremise)) {
            if (*v == var)
                theorem = specialization(theorem, term);
        }
        else { // в случае терма пустой путь будет означать ошибку
            Terms::Path usageOfVar = findVarFirstUsage(var,
                                                       static_cast<const Term*>(foralledPremise));
            if (usageOfVar.empty())
                theorem = moveQuantorIntoImpl(fT);
            else {
                const Terms* implement = term->get(usageOfVar);
                theorem = specialization(theorem, implement);
            }
        }
    }
    return modusPonens(term, theorem);
}

Terms* equalSubst(const Terms* term, const Terms* equality) {
    if (auto eq = dynamic_cast<const Term*>(equality))
        return term->replace(eq->arg(1), eq->arg(2));
    else
        return nullptr;
}

Term* generalization(const Terms* toGen, const Terms* x) {
    if (const auto* v = dynamic_cast<const Variable*>(x)) {
        if (const auto* tG = dynamic_cast<const Term*>(toGen)) {
            if (tG->free.find(*v) != tG->free.end())
                return new ForallTerm(v->clone(), tG->clone());
        }
    }
    else
        return nullptr;
}

Hierarchy* TermsBox::fromJson(const json& j, Node* parent)
{ return new TermsBox(parent, j.at("axiom")); }

Hierarchy* Inference::fromJson(const json& j, Node* parent)
{ return new Inference(parent, mkPath(j.at("premise1")),
                       mkPath(j.at("premise2")), infTyFromStr(j.at("type"))); }
