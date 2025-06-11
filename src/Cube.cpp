//
//  Cube.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 21.10.24.
//

#include <glad/glad.h>
#include "glm/gtc/matrix_transform.hpp"

#include "Shader.h"
#include "Camera.h"

#include "Cube.h"

void Cube::init()
{
    static const GLfloat vertices[] = {
        // front
        -0.5, -0.5,  0.5,
         0.5, -0.5,  0.5,
         0.5,  0.5,  0.5,
        -0.5,  0.5,  0.5,
        // back
        -0.5, -0.5, -0.5,
         0.5, -0.5, -0.5,
         0.5,  0.5, -0.5,
        -0.5,  0.5, -0.5
    };
    
    static const GLushort indices[] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 8, vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 3 * 12, indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/cube.glsl");
    
    position = glm::vec3(0);
    scale = glm::vec3(8);
    color = glm::vec4(1);
}

void Cube::draw(const Camera& camera) const
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    
    glm::mat4x4 mvp = camera.projection * camera.view * model;
    
    shader.bind();
    shader.setUniform("MVP", mvp);
    shader.setUniform("color", color);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
}

void WiredCube::init()
{
    static const GLfloat vertices[] = {
        // front
        -0.5, -0.5,  0.5,
         0.5, -0.5,  0.5,
         0.5,  0.5,  0.5,
        -0.5,  0.5,  0.5,
        // back
        -0.5, -0.5, -0.5,
         0.5, -0.5, -0.5,
         0.5,  0.5, -0.5,
        -0.5,  0.5, -0.5
    };
    
    static const GLushort indices[] = {
        0, 1,
        1, 2,
        2, 3,
        3, 0,
        
        4, 5,
        5, 6,
        6, 7,
        7, 4,
        
        0, 4,
        1, 5,
        2, 6,
        3, 7
    };
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 8, vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 2 * 12, indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/cube.glsl");
    
    position = glm::vec3(0);
    scale = glm::vec3(8);
    color = glm::vec4(1);
}

void WiredCube::draw(const Camera& camera) const
{
    glm::mat4 model(1.0f);
    
    model = glm::translate(model, position);
    model = glm::rotate(model, yaw, {0, 0, 1});
    model = glm::scale(model, scale);
    
    glm::mat4x4 mvp = camera.projection * camera.view * model;
    
    shader.bind();
    shader.setUniform("MVP", mvp);
    shader.setUniform("color", color);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
}

void ShadedCube::init()
{
    static const GLfloat positions[] = {
        // Передняя грань
        0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
        0.0, 1.0, 1.0,
        // Задняя грань
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        // Правая грань
        1.0, 0.0, 0.0,
        1.0, 1.0, 0.0,
        1.0, 1.0, 1.0,
        1.0, 0.0, 1.0,
        // Левая грань
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 1.0,
        0.0, 0.0, 1.0,
        // Верхняя грань
        0.0, 1.0, 1.0,
        1.0, 1.0, 1.0,
        1.0, 1.0, 0.0,
        0.0, 1.0, 0.0,
        // Нижняя грань
        0.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        1.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
    };

    GLfloat normals[] = {
        // Передняя грань
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
        // Задняя грань
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
        // Левая грань
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        // Правая грань
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
        // Верхняя грань
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
        // Нижняя грань
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
    };

    static const GLushort indices[] = {
        0, 1, 2, 2, 3, 0,    // Передняя грань
        4, 7, 6, 6, 5, 4,    // Задняя грань
        8, 9, 10, 10, 11, 8, // Левая грань
        12, 15, 14, 14, 13, 12, // Правая грань
        16, 17, 18, 18, 19, 16, // Верхняя грань
        20, 23, 22, 22, 21, 20  // Нижняя грань
    };

    
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_ARRAY_BUFFER, pbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    
    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, pbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/cube_shaded.glsl");
}

void ShadedCube::draw(const Camera& camera) const
{
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, scale);
    
    glm::mat4x4 mvp = camera.projection * camera.view * model;
    
    shader.bind();
    shader.setUniform("MVP", mvp);
    
    glm::vec4 color{0.5, 0.0, 0.5, 1.0};
    shader.setUniform("color", color);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
}
