//
//  Cube.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 21.10.24.
//

#pragma once

#include <glm/glm.hpp>
#include "Shader.h"

class Camera;

class Cube
{
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    Shader shader;
    
public:
    void init();
    void draw(const Camera& camera) const;
    
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec4 color;
};

class WiredCube
{
    unsigned int vbo;
    unsigned int ibo;
    unsigned int vao;
    Shader shader;
    
public:
    void init();
    void draw(const Camera& camera) const;
    
    glm::vec3 position;
    glm::vec3 scale;
    float yaw;
    
    glm::vec4 color;
};

class ShadedCube
{
    unsigned int pbo;
    unsigned int nbo;
    unsigned int ibo;
    unsigned int vao;
    Shader shader;
    
public:
    void init();
    void draw(const Camera& camera) const;
    
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec4 color;
};
