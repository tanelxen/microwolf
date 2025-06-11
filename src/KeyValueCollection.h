//
//  KeyValueCollection.hpp
//  KeyValueParser
//
//  Created by Fedor Artemenkov on 28.10.24.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

class KeyValueEntry
{
public:
    void debugPrint() const;
    std::unordered_map<std::string, std::string> properties;
    
    bool getIntValue(const std::string &name, int &value);
    bool getVec3Value(const std::string &name, glm::vec3 &value);
};

class KeyValueCollection
{
public:
    void initFromString(const std::string& input);
    void debugPrint() const;
    
    std::vector<KeyValueEntry> getAllWithKeyValue(const std::string& key, const std::string& value);
    
    
private:
    std::vector<KeyValueEntry> entries;
};

