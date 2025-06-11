//
// Created by Fedor Artemenkov on 05.07.2024.
//

#pragma once
#include <glm/glm.hpp>

//struct Ray
//{
//    glm::vec3 origin;
//    glm::vec3 dir;
//};
//
//struct Plane
//{
//    glm::vec3 normal;
//    float distance;
//};

class Camera
{
public:
    void setTransform(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& right, const glm::vec3& up);
    void setAspectRatio(float aspectRatio);
    
    glm::vec3 getPosition() const;
    
//    Ray getMousePosInWorld() const;
    
    glm::vec3 getForward() const { return m_forward; }

    glm::mat4x4 projection;
    glm::mat4x4 view;
    
    glm::mat4x4 weaponProjection;

private:
    glm::vec3 m_position;

    glm::vec3 m_forward = {0, 0, 0};
    glm::vec3 m_right = {0, 0, 0};
    glm::vec3 m_up = {0, 0, 0};
    
    float width;
    float height;
};
