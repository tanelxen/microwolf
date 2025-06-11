//
//  TextureAtlas.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 29.10.24.
//

#pragma once

#include "Q3BSPTypes.h"
#include <string>
#include <vector>

struct TextureTile
{
    int x, y;
    int width, height;
};

struct TextureAtlas
{
    ~TextureAtlas();
    
    void initFromQ3Lightmaps(const std::vector<tBSPLightmap>& lightmaps);
    void saveToPng(const std::string &filename);
    
    std::vector<TextureTile> tiles;
    int width, height;
    
    unsigned char* buffer;
};
