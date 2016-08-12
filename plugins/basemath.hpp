#ifndef BASEMATH_HPP
#define BASEMATH_HPP
class AbstractMathObject
{
public:
    AbstractMathObject() {}
    virtual ~AbstractMathObject() {}
};

class MathSymbol {
private:
    AbstractMathObject* const target;
public:
    MathSymbol(std::string);
};

class Expression : public AbstractMathObject
{

};

class Set;
class Predicat : public Expression
{
private:
    unsigned int arity;
public:
    Predicat();
    Predicat(std::string);
    Predicat(Set);
    ~Predicat();

    bool interpr(const AbstractMathObject**);
};

class Set : public AbstractMathObject
{
private:
    Predicat* condition;
public:
    Set() {}
    Set(Predicat _condition);
    ~Set();

    bool isBelongs(AbstractMathObject*);
};

/*class Set_category
{

};*/

#endif //BASEMATH_HPP