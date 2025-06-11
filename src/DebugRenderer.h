//
//  DebugRenderer.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 02.12.24.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

struct DebugLine
{
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 color;
    float lifeSpan;
};

struct DebugBBox
{
    glm::vec3 mins;
    glm::vec3 maxs;
    glm::vec3 color;
    float lifeSpan;
};

struct DebugRenderer
{
    void addLine(glm::vec3 start, glm::vec3 end, glm::vec3 color, float lifeSpan);
    void addBBox(glm::vec3 mins, glm::vec3 maxs, glm::vec3 color);
    
    void update(float dt);
    void draw(const class Camera& camera);
    
    static DebugRenderer& getInstance()
    {
        static DebugRenderer instance;
        return instance;
    }
    
private:
    DebugRenderer();
    
    std::vector<DebugLine> m_lines;
    std::vector<DebugBBox> m_boxes;
    
    bool isDirty = true;
};
