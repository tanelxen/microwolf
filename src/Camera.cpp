//
// Created by Fedor Artemenkov on 05.07.2024.
//

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

//struct Transform
//{
//    glm::vec3 position;
//    glm::vec3 rotation;
//};

void Camera::setAspectRatio(float aspectRatio)
{
    float worldFOV = glm::radians(55.0f);
    projection = glm::perspective(worldFOV, aspectRatio, 0.1f, 8192.0f);
    
    float weaponFOV = glm::radians(54.0f);
    weaponProjection = glm::perspective(weaponFOV, aspectRatio, 0.1f, 256.0f);
}

//Ray Camera::getMousePosInWorld() const
//{
//    double mouseX = 0;
//    double mouseY = 0;
//    glfwGetCursorPos(window, &mouseX, &mouseY);
//    
//    int screenWidth = 1;
//    int screenHeight = 1;
//    glfwGetWindowSize(window, &screenWidth, &screenHeight);
//    
//    double x_ndc = (2.0f * mouseX) / screenWidth - 1.0f;
//    double y_ndc = 1.0f - (2.0f * mouseY) / screenHeight;
//    
//    glm::vec4 nearPoint = {x_ndc, y_ndc, -1.0f, 1.0f};
//    glm::vec4 farPoint = {x_ndc, y_ndc, 1.0f, 1.0f};
//    
//    glm::mat4 viewProjInv = glm::inverse(projection * view);
//    
//    glm::vec4 nearWorld = viewProjInv * nearPoint;
//    glm::vec4 farWorld = viewProjInv * farPoint;
//    
//    nearWorld /= nearWorld.w;
//    farWorld /= farWorld.w;
//    
//    glm::vec3 origin = glm::vec3(nearWorld);
//    
//    glm::vec3 dir = glm::vec3(farWorld - nearWorld);
//    dir = glm::normalize(dir);
//    
//    return {.origin = origin, .dir = dir};
//}

void Camera::setTransform(const glm::vec3 &position, const glm::vec3 &forward, const glm::vec3 &right, const glm::vec3 &up)
{
    m_position = position;
    m_forward = forward;
    m_right = right;
    m_up = up;
    
    view = glm::lookAt(position, position + forward, up);
    m_position = position;
}

glm::vec3 Camera::getPosition() const
{
    return m_position;
}


