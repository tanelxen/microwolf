//
//  TextureAtlas.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 29.10.24.
//

#include "TextureAtlas.h"
#include <string.h> /* For memset() */

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb_image_write.h"

#define LIGHTMAP_SIZE 128
#define NUM_COMPONENTS 3

void TextureAtlas::initFromQ3Lightmaps(const std::vector<tBSPLightmap>& lightmaps)
{
    if (lightmaps.empty()) return;

    int gridSize = ceil(sqrt(lightmaps.size()));            // количество плиток по одной стороне
    int dim = gridSize * LIGHTMAP_SIZE;                     // ширина и высота атласа
    size_t tileRowSize = LIGHTMAP_SIZE * NUM_COMPONENTS;    // байтов в одной строке тайла (128 пикселей * 3 байта на пиксель)

    size_t bufferSize = dim * dim * NUM_COMPONENTS;         // полный размер атласа
    buffer = new unsigned char[bufferSize];
    memset(buffer, 0, bufferSize);

    this->width = this->height = dim;

    int xOffset = 0, yOffset = 0;

    for (int i = 0; i < lightmaps.size(); ++i)
    {
        TextureTile& tile = tiles.emplace_back();

        tile.x = xOffset;
        tile.y = yOffset;
        tile.width = LIGHTMAP_SIZE;
        tile.height = LIGHTMAP_SIZE;
        
        int targetX = tile.x;
        int targetY = tile.y;
        
        // Копируем изображение построчно
        for (int sourceY = 0; sourceY < tile.height; ++sourceY)
        {
            for (int sourceX = 0; sourceX < tile.width; ++sourceX)
            {
                int from = (sourceY * LIGHTMAP_SIZE * NUM_COMPONENTS) + (sourceX * NUM_COMPONENTS);
                int to = ((targetY + sourceY) * height * NUM_COMPONENTS) + ((targetX + sourceX) * NUM_COMPONENTS);
                
                for (int comp = 0; comp < NUM_COMPONENTS; ++comp)
                {
                    buffer[to + comp] = lightmaps[i].imageBits[from + comp];
                }
            }
        }

        // Переход к следующему тайлу вправо
        xOffset += LIGHTMAP_SIZE;

        // Если достигли края атласа, сбрасываем xOffset и переходим на следующую строку
        if (xOffset >= this->width)
        {
            xOffset = 0;
            yOffset += LIGHTMAP_SIZE;
        }
    }
}

void TextureAtlas::saveToPng(const std::string &filename)
{
    stbi_write_png(filename.c_str(), width, height, NUM_COMPONENTS, buffer, width * NUM_COMPONENTS);
}

TextureAtlas::~TextureAtlas()
{
    if (buffer != nullptr)
    {
        delete [] buffer;
    }
}
