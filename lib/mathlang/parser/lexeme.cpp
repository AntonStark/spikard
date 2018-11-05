//
// Created by anton on 30.10.18.
//

#include "lexeme.hpp"

namespace Parser2
{

LexemStorage::CatCode LexemStorage::_catCode(std::string category) {
    for (int i = 0; i < _catNames.size(); ++i)
        if (_catNames[i] == category)
            return i;
    _catNames.push_back(category);
    return (_catNames.size() - 1);
}

LexemStorage::Id LexemStorage::_store(const std::string& cmd, unsigned char catCode) {
    auto search = _dictionary.find(cmd);
    if (search != _dictionary.end()) {              // в случае, когда такая команда уже определена и ...
        unsigned char oldCat = _catIndex[search->second];
        if (catCode != 0 && oldCat != catCode)      // категория задана явно и не совпадает ...
            _catIndex[search->second] = catCode;    // просто изменяем категорию команды
        return search->second;
    } else {
        Id id = _index.size();
        _index.push_back(cmd);
        _dictionary[cmd] = id;
        _catIndex.push_back(catCode);
        return id;
    }
}

}
