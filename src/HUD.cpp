//
//  HUD.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 02.12.24.
//

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "HUD.h"

static unsigned int vbo;
static unsigned int vao;

static Shader shader;
static glm::mat4x4 projection;

glm::vec2 size = glm::vec2(5);
glm::vec2 position = glm::vec2(0);

void HUD::init()
{
    float vertices[] = {
        // pos
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
    
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/basic_flat.glsl");
}

void HUD::resize(int width, int height)
{
    float aspect = (float)width/height;
    projection = glm::ortho(-0.5f * width, 0.5f * width, -0.5f * height, 0.5f * height, 0.0f, 1.0f);
//    projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
}

void HUD::draw()
{
    shader.bind();
    
//    projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, 0.0f, 1.0f);
    shader.setUniform("uProjMatrix", projection);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    
//    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    
    model = glm::scale(model, glm::vec3(size, 1.0f));
    
    shader.setUniform("uModelMatrix", model);
    
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
