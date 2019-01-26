//
// Created by anton on 07.01.19.
//

#include "abstract_name.hpp"

/**
 * за хранение имён отвечает NSI. поэтому стоит связать работу по созданию имени с методом его регистрации
 * то есть AbtractName создаётся и удаляется в NSI, наружу уходит только share_pointer
 * todo ^^
 */
NamedEntity::NamedEntity(const AbstractName* name)
    : _name(name) {}
bool NamedEntity::operator==(const NamedEntity& one) const { return (_name == one._name); }
bool NamedEntity::operator<(const NamedEntity& other) const { return (_name < other._name); }
const AbstractName* const NamedEntity::getName() const { return _name; }
