//
//  Q3LightGrid.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 25.11.24.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Q3LightGrid
{
    void init(const struct Q3BSPAsset& q3bsp);
    
    void getValue(const glm::vec3& pos, glm::vec3& ambient, glm::vec3& color, glm::vec3& dir) const;
    
private:
    glm::vec3 maxs;
    glm::vec3 mins;
    
    int lightVolSizeX;
    int lightVolSizeY;
    int lightVolSizeZ;
    
    std::vector<glm::vec3> ambients;
    std::vector<glm::vec3> directionals;
    std::vector<glm::vec3> dirs;
    
    int indexForCell(int x, int y, int z) const;
};
