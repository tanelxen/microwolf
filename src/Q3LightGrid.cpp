//
//  Q3LightGrid.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.11.24.
//

#include <algorithm>

#include "Q3LightGrid.h"
#include "Q3BSPAsset.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define radians(deg) ((deg) * (M_PI / 180.0f))

using std::min, std::max;

void Q3LightGrid::init(const Q3BSPAsset& q3bsp)
{
    maxs = q3bsp.m_models[0].max;
    mins = q3bsp.m_models[0].min;
    
    lightVolSizeX = floor(maxs.x / 64) - ceil(mins.x / 64) + 1;
    lightVolSizeY = floor(maxs.y / 64) - ceil(mins.y / 64) + 1;
    lightVolSizeZ = floor(maxs.z / 128) - ceil(mins.z / 128) + 1;
    
    ambients.resize(q3bsp.m_lightVolumes.size());
    directionals.resize(q3bsp.m_lightVolumes.size());
    dirs.resize(q3bsp.m_lightVolumes.size());
    
    for (int i = 0; i < q3bsp.m_lightVolumes.size(); ++i)
    {
        const auto& cell = q3bsp.m_lightVolumes[i];
        
        ambients[i].x = cell.ambient[0] / 255.0;
        ambients[i].y = cell.ambient[1] / 255.0;
        ambients[i].z = cell.ambient[2] / 255.0;
        
        directionals[i].x = cell.directional[0] / 255.0;
        directionals[i].y = cell.directional[1] / 255.0;
        directionals[i].z = cell.directional[2] / 255.0;
        
        float lattitude = radians(cell.dir[1] * 360.0 / 255.0);
        float longitude = radians(cell.dir[0] * 360.0 / 255.0);
        
        dirs[i] = {
            cos(lattitude) * sin(longitude),
            sin(lattitude) * sin(longitude),
            cos(longitude)
        };
    }
}

void Q3LightGrid::getValue(const glm::vec3& pos, glm::vec3& ambient, glm::vec3& color, glm::vec3& dir) const
{
    if (ambients.empty()) return;
    
    int cellX = floor(pos.x / 64) - ceil(mins.x / 64) + 1;
    int cellY = floor(pos.y / 64) - ceil(mins.y / 64) + 1;
    int cellZ = floor(pos.z / 128) - ceil(mins.z / 128) + 1;
    
    int index = indexForCell(cellX, cellY, cellZ);

    if (index >= ambients.size()) return;
    
    ambient = ambients[index];
    color = directionals[index];
    dir = dirs[index];
}

int Q3LightGrid::indexForCell(int x, int y, int z) const
{
    int cellX = min(max(x, 0), lightVolSizeX);
    int cellY = min(max(y, 0), lightVolSizeY);
    int cellZ = min(max(z, 0), lightVolSizeZ);
    
    return cellX + cellY * lightVolSizeX + cellZ * lightVolSizeX * lightVolSizeY;
}
