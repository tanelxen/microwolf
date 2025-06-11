//
//  KeyValueCollection.cpp
//  KeyValueParser
//
//  Created by Fedor Artemenkov on 28.10.24.
//

#include "KeyValueCollection.h"
#include <iostream>
#include <sstream>

void KeyValueCollection::initFromString(const std::string &input)
{
    size_t pos = 0;

    while (pos < input.size())
    {
        // Ищем начало блока '{'
        pos = input.find('{', pos);
        if (pos == std::string::npos) break;

        KeyValueEntry current;
        pos++;  // Сдвигаемся после '{'

        // Ищем конец блока '}'
        size_t endPos = input.find('}', pos);
        if (endPos == std::string::npos) break;

        // Парсим содержимое между '{' и '}'
        while (pos < endPos)
        {
            // Ищем первую кавычку ключа
            size_t keyStart = input.find('"', pos);
            if (keyStart == std::string::npos || keyStart >= endPos) break;
            size_t keyEnd = input.find('"', keyStart + 1);
            if (keyEnd == std::string::npos || keyEnd >= endPos) break;

            std::string key = input.substr(keyStart + 1, keyEnd - keyStart - 1);

            // Ищем первую кавычку значения
            size_t valueStart = input.find('"', keyEnd + 1);
            if (valueStart == std::string::npos || valueStart >= endPos) break;
            size_t valueEnd = input.find('"', valueStart + 1);
            if (valueEnd == std::string::npos || valueEnd >= endPos) break;

            std::string value = input.substr(valueStart + 1, valueEnd - valueStart - 1);

            // Сохраняем ключ-значение в текущем объекте
            current.properties[key] = value;

            // Перемещаемся к следующей паре
            pos = valueEnd + 1;
        }

        // Добавляем текущий объект в список
        entries.push_back(current);
        pos = endPos + 1;  // Переходим после '}'
    }
}

void KeyValueCollection::debugPrint() const
{
    for (const auto& entry : entries)
    {
        entry.debugPrint();
    }
}

void KeyValueEntry::debugPrint() const
{
    std::cout << "{\n";
    for (const auto& [key, value] : this->properties) {
        std::cout << "  " << key << ": \"" << value << "\"\n";
    }
    std::cout << "}\n";
}

bool KeyValueEntry::getIntValue(const std::string &name, int &value)
{
    auto property = properties.find(name);
    
    if (property == properties.end()) return false;
    
    std::string valueStr = property->second;
    value = atoi(valueStr.c_str());
    
    return true;
}

bool KeyValueEntry::getVec3Value(const std::string &name, glm::vec3 &value)
{
    auto property = properties.find(name);
    
    if (property == properties.end()) return false;
    
    std::string valueStr = property->second;
    
    std::istringstream stream(valueStr);
    stream >> value.x >> value.y >> value.z;
    
    return true;
}

std::vector<KeyValueEntry> KeyValueCollection::getAllWithKeyValue(const std::string &key, const std::string &value)
{
    std::vector<KeyValueEntry> result;
    
    for (const auto& entry : entries)
    {
        auto it = entry.properties.find(key);  // Ищем ключ в текущем объекте
        
        if (it != entry.properties.end() && it->second == value)
        {
            // Ключ найден, и значение совпадает
            result.push_back(entry);
        }
    }
    
    return result;
}
