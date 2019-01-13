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
};


#endif //SPIKARD_NAMESTYPE_HPP
