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

#include "RenderDevice.h"

struct Q3ShaderStage
{
    std::unordered_map<std::string, std::string> parameters;
    
    bool getMap(std::string& map)
    {
        {
            auto it = parameters.find("map");
            
            if (it != parameters.end())
            {
                map = it->second;
                return true;
            }
        }
        
        {
            auto it = parameters.find("animMap");
            
            if (it != parameters.end())
            {
                map = it->second;
                return true;
            }
        }
        
        return false;
    }
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
    
    bool getBaseTextureName(const std::string& shaderName, std::string& textureName, BlendMode& mode) const;
    
private:
    void readFile(const std::string& filename);
    
public:
    std::unordered_map<std::string, Q3ShaderItem> entries;
};
