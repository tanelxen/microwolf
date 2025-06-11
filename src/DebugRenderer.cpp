//
//  DebugRenderer.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 02.12.24.
//

#include <glad/glad.h>
#include "Camera.h"
#include "Shader.h"

#include "DebugRenderer.h"

static unsigned int vbo;
static unsigned int vao;
static Shader shader;

DebugRenderer::DebugRenderer()
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    shader.init("assets/shaders/cube.glsl");
}

void DebugRenderer::addLine(glm::vec3 start, glm::vec3 end, glm::vec3 color, float lifeSpan)
{
    auto& line = m_lines.emplace_back();
    line.start = start;
    line.end = end;
    line.color = color;
    line.lifeSpan = lifeSpan;
    
    isDirty = true;
}

void DebugRenderer::addBBox(glm::vec3 mins, glm::vec3 maxs, glm::vec3 color)
{
    
}

void DebugRenderer::update(float dt)
{
    int erasedCount = 0;
    
    for (auto it = m_lines.begin(); it != m_lines.end();)
    {
        it->lifeSpan -= dt;
        
        if (it->lifeSpan <= 0.0f) {
            it = m_lines.erase(it);
            erasedCount++;
        } else {
            ++it;
        }
    }
    
    isDirty |= erasedCount > 0;
}

void DebugRenderer::draw(const Camera& camera)
{
    if (m_lines.empty()) return;
    
    glm::mat4x4 mvp = camera.projection * camera.view;
    
    shader.bind();
    shader.setUniform("MVP", mvp);
    shader.setUniform("color", glm::vec4(1));
    
    if (isDirty)
    {
        std::vector<float> vertexData;
        
        for (const auto& line : m_lines)
        {
            vertexData.insert(vertexData.end(), {
                line.start.x, line.start.y, line.start.z,
                line.end.x, line.end.y, line.end.z
            });
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);
        
        isDirty = false;
    }
    
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_lines.size() * 2));
}

