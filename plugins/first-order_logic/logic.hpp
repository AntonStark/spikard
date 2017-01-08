//
// Created by anton on 03.12.16.
//

#ifndef TEST_BUILD_LOGIC_HPP
#define TEST_BUILD_LOGIC_HPP

#include <string>
#include <iostream>
#include <list>
#include <stdexcept>
#include <memory>
#include <set>
#include <map>

class Printable
{
public:
    virtual ~Printable() {}

    virtual void print(std::ostream& out = std::cout) const = 0;
    friend std::ostream& operator<<(std::ostream& os, const Printable& pr);
};

class Named
{
private:
    const std::string name;
public:
    Named(const std::string& _name) : name(_name) {}
    ~Named() {}
    Named(const Named& one) : name(one.name) {}

    std::string getName() const { return name; }
    bool operator== (const Named& one) const;
};

class Symbol : public virtual Printable, public Named
{
public:
    Symbol(const std::string& _name) : Named(_name) {}
    virtual ~Symbol() {}
    Symbol(const Symbol& one) : Named(one) {}

    void print(std::ostream& out = std::cout) const override;
    bool operator== (const Symbol& one) const;
};

class Map
{
private:
    const unsigned arity;
public:
    Map(unsigned _arity) : arity(_arity) {}
    virtual ~Map() {}
    Map(const Map& one) : arity(one.arity) {}

    unsigned getArity() const { return arity; }
    bool operator== (const Map& one) const;
};

/*=====================================================*/
/*=====================================================*/
/*=====================================================*/
class Signature;
class Predicate : public Symbol, public Map
{
private:
//    std::shared_ptr<Signature> sigma;
    Predicate(const std::string& _name, unsigned _arity/*, Signature* _sigma = nullptr*/)
            : Symbol(_name), Map(_arity)/*, sigma(_sigma)*/ {}
    friend class Signature;
public:
    virtual ~Predicate() {}
    Predicate(const Predicate& one)
            : Symbol(one), Map(one)/*, sigma(one.sigma)*/ {}

    bool operator== (const Predicate& one) const;
    typedef std::shared_ptr<Predicate> sh_p;
};

class Function : public Symbol, public Map
{
private:
    Function(const std::string& _name, unsigned _arity)
            : Symbol(_name), Map(_arity) {}
    friend class Signature;
public:
    virtual ~Function() {}
    Function(const Function& one)
            : Symbol(one), Map(one) {}

    bool operator== (const Function& one) const;
    typedef std::shared_ptr<Function> sh_p;
};


//TODO Перенести Signature сюда, а всё начиная с Modifier туда. И будет signature.hpp и statements.hpp
class Variable;
class Terms : public virtual Printable
{
protected:
    //TODO нужно использовать weak_ptr, иначе двойное удаление
    //TODO нужно аккуратно управлять созданием (и хранением) всех классов. Позапрещать конструкторы и пусть за всё отвечает сигнатура?
    std::set<std::shared_ptr<Variable> > vars;
public:
    virtual ~Terms() {}
    virtual Terms* clone() const = 0;
    virtual bool isVariable() const { return false; }
    std::set<std::shared_ptr<Variable> > getVars() const { return vars; };
};

class Variable : public Terms, public Symbol
{
public:
    Variable(const std::string& _name) : Symbol(_name) {/*vars.emplace(this);*/}
    Variable(const Variable& one) : Symbol(one) {}
    virtual ~Variable() {}

    Variable* clone() const override { return (new Variable(*this)); }
    bool isVariable() const override { return true; }
};

class Constant : public Terms, public Symbol
{
private:
    Constant(const std::string& _name) : Symbol(_name) {}
    friend class Signature;
public:
    virtual ~Constant() {}
    Constant(const Constant& one) : Symbol(one) {}

    Constant* clone() const override { return (new Constant(*this)); }
    typedef std::shared_ptr<Constant> sh_p;
};

class ParenSymbol : public virtual Printable
{
public:
    //TODO следует использовать weak_ptr, но это создаёт проблемы в случае [_args]=std::list<Terms*>
    //TODO vars как объединение vars всех аргументов.
    typedef std::list<std::shared_ptr<Terms> > TermsList;
private:
    TermsList args;
    std::set<std::shared_ptr<Variable> > vars;
protected:
    void argCheck(Map* f, std::list<Terms*> _args);
    void argCheck(std::shared_ptr<Map> f, TermsList _args);
public:
    ParenSymbol(std::list<Terms*> _args);
    ParenSymbol(TermsList _args);
    ParenSymbol(const ParenSymbol& one) : args(one.args), vars(one.vars) {}
    virtual ~ParenSymbol() {}

    virtual void print(std::ostream& out = std::cout) const override;
    class nArg_arity_error;
};

class Term : public Terms, protected Function, protected ParenSymbol
{
public:
    Term(Function* f, std::list<Terms*> _args)
            : Function(*f), ParenSymbol(_args) { argCheck(f, _args); }
    Term(std::shared_ptr<Function> f, TermsList _args)
            : Function(*f.get()), ParenSymbol(_args) { argCheck(f, _args); }
    Term(const Term& one)
            : Function(one), ParenSymbol(one) {}
    virtual ~Term() {}

    Term* clone() const override { return (new Term(*this)); }
    virtual void print(std::ostream& out = std::cout) const override;
};

class Signature
{
private:
    std::map<std::string, std::shared_ptr<Predicate> > R;
    std::map<std::string, std::shared_ptr<Function> > F;
    std::map<std::string, std::shared_ptr<Constant> > C;

    class sym_doubling;
//    class sym_exists;
    class no_sym;
public:
    Signature() {}
    Signature(std::list<std::pair<std::string, unsigned> > _R,
              std::list<std::pair<std::string, unsigned> > _F,
              std::list<std::string> _C);
    ~Signature() {}

    bool isPredName(const std::string &name) const
    { return (R.find(name) != R.end()); }
    bool isFuncName(const std::string &name) const
    { return (F.find(name) != F.end()); }
    bool isConsName(const std::string &name) const
    { return (C.find(name) != C.end()); }

    enum class nameT {predicate, function, constant, none};
    nameT checkName(const std::string& name) const;

    unsigned arity(const std::string& name) const;

    void addP(const std::string& name, unsigned arity);
    void addF(const std::string& name, unsigned arity);
    void addC(const std::string& name);

    Predicate::sh_p getP(const std::string& name) const;
    Function::sh_p getF(const std::string& name) const;
    Constant::sh_p getC(const std::string& name) const;

    unsigned long maxLength(nameT type) const;
};

/*const Atom& Predicate::operator() (std::vector<Terms*> _args)
{ return Atom(this, _args); }
const Term& Function ::operator() (std::vector<Terms*> _args)
{ return Term(this, _args); }*/

#endif //TEST_BUILD_LOGIC_HPP
