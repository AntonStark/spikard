//
// Created by anton on 07.01.19.
//

#ifndef SPIKARD_NAMESTYPE_HPP
#define SPIKARD_NAMESTYPE_HPP


#include <string>

class AbstractName
{
    virtual void fromStr(const std::string& name) = 0;
public:
    virtual std::string toStr() const = 0;
    virtual bool operator==(const AbstractName& other) const = 0;
};

class NamedEntity
{
private:
    const AbstractName* _name; // todo использовать shred_pointer полученный из NSI
public:
    virtual ~NamedEntity() = default;
    NamedEntity(const AbstractName* name);

    bool operator== (const NamedEntity& one) const;
    bool operator< (const NamedEntity& other) const;
    const AbstractName* const getName() const;
};

#endif //SPIKARD_NAMESTYPE_HPP
