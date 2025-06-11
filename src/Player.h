//
//  Player.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 03.11.24.
//

#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Footsteps.h"

class PlayerMovement;
struct GoldSrcModelInstance;

class Player
{
public:
    Player(PlayerMovement* movement);
    
    void update(float dt);
    
    glm::vec3 position = {0, 128, 256};
    
    glm::vec3 forward = {0, 0, 0};
    glm::vec3 right = {0, 0, 0};
    glm::vec3 up = {0, 0, 0};
    
    float pitch = -0.5;
    float yaw = -1.57;
    
    std::unique_ptr<GoldSrcModelInstance> m_pModelInstance;
    
private:
    PlayerMovement* m_pMovement;
    glm::vec3 velocity = {0, 0, 0};
    Footsteps m_footsteps;
};

class PlayerDebug
{
public:
    PlayerDebug();
    
    void update(float dt);
    
    glm::vec3 position = {0, 128, 256};
    
    glm::vec3 forward = {0, 0, 0};
    glm::vec3 right = {0, 0, 0};
    glm::vec3 up = {0, 0, 0};
    
    float pitch = -0.5;
    float yaw = -1.57;

};
