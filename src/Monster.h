//
//  Monster.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 28.11.24.
//

#pragma once

#include <memory>

struct GoldSrcModelInstance;

class Monster
{
public:
    void update(float dt);
    
    glm::vec3 position;
    float yaw;
    
    std::unique_ptr<GoldSrcModelInstance> m_pModelInstance;
};
