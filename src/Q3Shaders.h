//
//  Quake3Shaders.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 23.11.24.
//

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct Q3ShaderStage
{
    std::unordered_map<std::string, std::string> parameters;
};

struct Q3ShaderItem
{
    std::string name;
    std::unordered_map<std::string, std::string> parameters;
    std::vector<Q3ShaderStage> stages;
};

class Quake3Shaders
{
public:
    void initFromDir(const std::string& dir);
    
    bool getBaseTextureName(std::string& textureName, const std::string& shaderName) const;
    
private:
    void readFile(const std::string& filename);
    
public:
    std::unordered_map<std::string, Q3ShaderItem> entries;
};
