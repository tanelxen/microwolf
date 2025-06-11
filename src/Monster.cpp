//
//  Monster.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 28.11.24.
//

#include <glm/glm.hpp>

#include "Monster.h"
#include "StudioRenderer.h"

void Monster::update(float dt)
{
    m_pModelInstance->animator.update(dt);
    
    m_pModelInstance->position = position + glm::vec3{0, 0, -24};
    m_pModelInstance->yaw = yaw;
}
