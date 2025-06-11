//
//  PlayerMovement.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 02.11.24.
//

#pragma once
#include <glm/glm.hpp>

class Q3BspCollision;

class PlayerMovement
{
public:
    PlayerMovement(Q3BspCollision* collision);
    
    void setTransform(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& right, const glm::vec3& up);
    void setInputMovement(int forward, int right, bool jump);
    
    void update(float dt);
    
    glm::vec3 getPosition() const { return m_position; };
    
    bool isWalk() const { return isGrounded && glm::length(velocity) > 0.1; };
    
    int getSurfaceFlags() const { return surfaceFlags; }
    
private:
    Q3BspCollision* m_pCollision;
    
private:
    void apply_inputs(float dt);
    void apply_air_control(const glm::vec3& direction, float wishspeed, float dt);
    void apply_acceleration(const glm::vec3& direction, float wishspeed, float acceleration, float dt);
    void apply_friction(float dt);
    void apply_jump();
    
    void trace_ground();
    void step_slide(bool gravity, float dt);
    bool slide(bool gravity, float dt);
    
    glm::vec3 m_position;
    glm::vec3 m_forward;
    glm::vec3 m_right;
    glm::vec3 m_up;
    
    float m_forwardmove;
    float m_rightmove;
    
    glm::vec3 velocity;
    
    glm::vec3 ground_normal;
    int surfaceFlags;
    bool isGrounded;
    
    int m_movementBits;
};
