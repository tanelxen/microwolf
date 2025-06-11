//
//  StudioRenderer.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 19.11.24.
//

#include <glm/gtc/matrix_transform.hpp>

#include "StudioRenderer.h"
#include "Camera.h"

#include "Q3LightGrid.h"

StudioRenderer::StudioRenderer()
{
    m_shader.init("assets/shaders/goldsrc_model.glsl");
}

void StudioRenderer::queue(GoldSrcModelInstance* inst)
{
    m_renderQueue.push_back(inst);
}

void StudioRenderer::queueViewModel(GoldSrcModelInstance* inst)
{
    m_renderQueueView.push_back(inst);
}

void StudioRenderer::drawRegular(const Camera* camera, const Q3LightGrid* lightGrid)
{
    m_shader.bind();
    
    for (const auto inst : m_renderQueue)
    {
        glm::mat4 model = glm::mat4(1);
        
        model = glm::translate(model, inst->position);
        model = glm::rotate(model, inst->yaw, glm::vec3(0, 0, 1));
        
        glm::vec3 ambient = glm::vec3{1};
        glm::vec3 color = glm::vec3{1};
        glm::vec3 dir = glm::vec3{0};
        
        if (lightGrid != nullptr)
        {
            glm::vec3 pos = inst->position;
            pos.z += 24;
            
            lightGrid->getValue(pos, ambient, color, dir);
        }
        
        m_shader.setUniform("u_ambient", ambient);
        m_shader.setUniform("u_color", color);
        m_shader.setUniform("u_dir", dir);
        
        // Model matrix uses for rotate normals
        m_shader.setUniform("uModel", model);
        
        glm::mat4 mvp = camera->projection * camera->view * model;
        m_shader.setUniform("uMVP", mvp);
        
        
        m_shader.setUniform("uBoneTransforms", inst->animator.transforms);
        
        inst->m_pmodel->mesh.draw();
    }
    
    m_renderQueue.clear();
}

void StudioRenderer::drawViewModels(const Camera* camera, const Q3LightGrid* lightGrid)
{
    m_shader.bind();
    
    glm::mat4 quakeToGL = {
         0,  0, -1,   0,
        -1,  0,  0,   0,
         0,  1,  0,   0,
         0,  0,  0.5, 1
    };
    
    for (const auto inst : m_renderQueueView)
    {
        
        glm::vec3 ambient = glm::vec3{1};
        glm::vec3 color = glm::vec3{1};
        glm::vec3 dir = glm::vec3{0};
        
        if (lightGrid != nullptr)
        {
            glm::vec3 pos = camera->getPosition();
            
            lightGrid->getValue(pos, ambient, color, dir);
        }
        
        dir = glm::mat3(camera->view) * dir;
        
        m_shader.setUniform("u_ambient", ambient);
        m_shader.setUniform("u_color", color);
        m_shader.setUniform("u_dir", dir);
        
        // Model matrix uses for rotate normals
        m_shader.setUniform("uModel", quakeToGL);
        
        glm::mat4 mvp = camera->weaponProjection * quakeToGL;
        m_shader.setUniform("uMVP", mvp);
        
        m_shader.setUniform("uBoneTransforms", inst->animator.transforms);
        
        inst->m_pmodel->mesh.draw();
    }
    
    m_renderQueueView.clear();
}

GoldSrcModel* StudioRenderer::makeModel(const std::string& filename)
{
    auto [it, inserted] = m_cache.try_emplace(filename);
    
    GoldSrcModel& model = it->second;
    
    if (inserted)
    {
        GoldSrc::Model asset;
        asset.loadFromFile(filename);
        
        model.mesh.init(asset);
        model.animation.sequences = asset.sequences;
        model.animation.bones = asset.bones;
    }
    else
    {
        printf("Use cached model for %s\n", filename.c_str());
    }
    
    return &model;
}

std::unique_ptr<GoldSrcModelInstance> StudioRenderer::makeModelInstance(const std::string& filename)
{
    auto inst = std::make_unique<GoldSrcModelInstance>();
    
    inst->m_pmodel = makeModel(filename);
    inst->animator.m_pAnimation = &(inst->m_pmodel->animation);
    
    return inst;
}

