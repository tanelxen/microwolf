//
//  Player.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 03.11.24.
//

#include <GLFW/glfw3.h>

#include "Player.h"
#include "PlayerMovement.h"

#include "Input.h"
#include "Application.h"

#include "GoldSrcModel.h"

static const float kMouseSense = 0.15;

Player::Player(PlayerMovement *movement) : m_pMovement(movement)
{
    m_footsteps.init();
}

void Player::update(float dt)
{
    if (m_pMovement == nullptr) return;

    yaw -= Input::getMouseOffsetX() * kMouseSense * dt;
    pitch += Input::getMouseOffsetY() * kMouseSense * dt;
    
    if (pitch > 1.5) pitch = 1.5;
    else if (pitch < -1.5) pitch = -1.5;

    
    forward.x = cos(pitch) * cos(yaw);
    forward.y = cos(pitch) * sin(yaw);
    forward.z = -sin(pitch);
    forward = glm::normalize(forward);

    glm::vec3 unit_up = {0, 0, 1};

    right = glm::cross(forward, unit_up);
    right = glm::normalize(right);

    up = glm::cross(right, forward);
    up = glm::normalize(up);

    
    float forwardmove = 0;
    float rightmove = 0;

    if (Input::isKeyPressed(KEY_W))
    {
        forwardmove += 1;
    }

    if (Input::isKeyPressed(KEY_S))
    {
        forwardmove -= 1;
    }

    if (Input::isKeyPressed(KEY_D))
    {
        rightmove += 1;
    }

    if (Input::isKeyPressed(KEY_A))
    {
        rightmove -= 1;
    }
    
    bool jump = Input::isKeyPressed(KEY_SPACE);
    
    m_pMovement->setTransform(position, forward, right, up);
    m_pMovement->setInputMovement(forwardmove, rightmove, jump);
    m_pMovement->update(dt);
    
    position = m_pMovement->getPosition();
    
    m_pModelInstance->parent_position = position;
    m_pModelInstance->parent_yaw = yaw;
    m_pModelInstance->hasParent = true;
    
    m_pModelInstance->position = glm::vec3(-0.5, 0, 40);
    m_pModelInstance->yaw = 0;
    m_pModelInstance->pitch = pitch;
    
    m_pModelInstance->animator.update(dt);
    
    m_footsteps.update(dt, m_pMovement);
}

PlayerDebug::PlayerDebug()
{
    
}

void PlayerDebug::update(float dt)
{
    if (Input::isLeftMouseButtonPressed())
    {
        Application::setCursorEnabled(false);
        
        yaw -= Input::getMouseOffsetX() * kMouseSense * dt;
        pitch += Input::getMouseOffsetY() * kMouseSense * dt;
    }
    else
    {
        Application::setCursorEnabled(true);
    }
    
    if (pitch > 1.5) pitch = 1.5;
    else if (pitch < -1.5) pitch = -1.5;
    
    
    forward.x = cos(pitch) * cos(yaw);
    forward.y = cos(pitch) * sin(yaw);
    forward.z = -sin(pitch);
    forward = glm::normalize(forward);
    
    glm::vec3 unit_up = {0, 0, 1};
    
    right = glm::cross(forward, unit_up);
    right = glm::normalize(right);
    
    up = glm::cross(right, forward);
    up = glm::normalize(up);
    
    
    float forwardmove = 0;
    float rightmove = 0;
    float upmove = 0;
    
    if (Input::isKeyPressed(KEY_W))
    {
        forwardmove += 1;
    }
    
    if (Input::isKeyPressed(KEY_S))
    {
        forwardmove -= 1;
    }
    
    if (Input::isKeyPressed(KEY_D))
    {
        rightmove += 1;
    }
    
    if (Input::isKeyPressed(KEY_A))
    {
        rightmove -= 1;
    }
    
    if (Input::isKeyPressed(KEY_SPACE))
    {
        upmove += 1;
    }
    
    if (Input::isKeyPressed(GLFW_KEY_C))
    {
        upmove -= 1;
    }
    
//    bool jump = Input::isKeyPressed(KEY_SPACE);
    
    static float cl_forwardspeed = 400.0;
    static float cl_sidespeed = 350.0;
    
    glm::vec3 direction = {0, 0, 0};
    direction += forward * forwardmove * cl_forwardspeed;
    direction += right * rightmove * cl_forwardspeed;
    direction += up * upmove * cl_forwardspeed;
    
    position += direction * dt;
}
