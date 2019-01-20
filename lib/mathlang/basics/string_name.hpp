//
// Created by anton on 20.01.19.
//

#ifndef SPIKARD_STRING_NAME_HPP
#define SPIKARD_STRING_NAME_HPP

#include "../consepts/abstract_name.hpp"

class StringName : public AbstractName
{
private:
    std::string _name;
    void fromStr(const std::string& name) override;
public:
    StringName(const std::string& name)
    { fromStr(name); }

    std::string toStr() const override;
};

#endif //SPIKARD_STRING_NAME_HPP
