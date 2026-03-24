//
//  RenderDevice.cpp
//  microwolf
//
//  Created by Fedor Artemenkov on 19.03.26.
//

#include "RenderDevice.h"

#include <stdio.h>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Camera.h"

namespace RenderDevice {
    
    std::vector<RenderCommand> opaquePass;
    std::vector<RenderCommand> transparentPass;
    
    static Shader q3bsp_shader;
    static Shader studio_shader;
    
    void init()
    {
        gladLoadGL();
        
        const unsigned char* version = glGetString(GL_VERSION);
        printf("version: %s\n", version);
        
        const unsigned char* device = glGetString(GL_RENDERER);
        printf("device: %s\n", device);
        
        glEnable(GL_FRAMEBUFFER_SRGB);
        glFrontFace(GL_CCW);
        
        q3bsp_shader.init("assets/shaders/q3bsp_world.glsl");
        q3bsp_shader.bind();
        glUniform1i(glGetUniformLocation(q3bsp_shader.program, "s_bspTexture"), 0);
        glUniform1i(glGetUniformLocation(q3bsp_shader.program, "s_bspLightmap"), 1);
        q3bsp_shader.unbind();
        
        studio_shader.init("assets/shaders/goldsrc_skinned.glsl");
    }
    
    handle_t makeTexture(int width, int height, bool hasAlpha, bool standard, const void* data)
    {
        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        GLenum internalformat;
        
        if (standard) {
            internalformat = hasAlpha ? GL_SRGB_ALPHA : GL_SRGB;
        } else {
            internalformat = hasAlpha ? GL_RGBA : GL_RGB;
        }
        
        GLenum format = hasAlpha ? GL_RGBA : GL_RGB;
        
        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return id;
    }
    
    handle_t makeVertexBuffer(long size, void* data)
    {
        GLuint id;
        
        glGenBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        return id;
    }
    
    handle_t makeIndexBuffer(long size, void* data)
    {
        GLuint id;
        
        glGenBuffers(1, &id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        return id;
    }
    
    handle_t makeVertexArray(handle_t vertexBuffer, handle_t indexBuffer, const VertexLayout& layout)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        
        GLuint id;
        glGenVertexArrays(1, &id);
        glBindVertexArray(id);
        
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        
        for (int i = 0; i < layout.attributes.size(); i++)
        {
            auto& attribute = layout.attributes[i];
            
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(i, attribute.count, attribute.type, attribute.normalized, layout.stride, (void*)attribute.offset);
        }
        
        glBindVertexArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        return id;
    }
    
    void submit(RenderCommand cmd)
    {
        if (cmd.blendMode == BlendMode::Transparent || cmd.blendMode == BlendMode::Add)
        {
            transparentPass.push_back(cmd);
        }
        else
        {
            opaquePass.push_back(cmd);
        }
    }
    
    void draw(glm::mat4& viewProj, std::vector<RenderCommand>& commands)
    {
        handle_t lastVAO = -1;
        handle_t lastIBO = -1;
        handle_t lastProgram = -1;
        handle_t lastBaseTexture = -1;
        handle_t lastLightmapTexture = -1;
        
        for (auto cmd : commands)
        {
            Shader* shader;
            
            switch (cmd.pipeline)
            {
                
                case PipelineType::World:
                    shader = &q3bsp_shader;
                    break;
                    
                case PipelineType::Skinned:
                    shader = &studio_shader;
                    
                    break;
            }
            
            if (lastProgram != shader->program)
            {
                shader->bind();
                lastProgram = shader->program;
            }
            
            glm::mat4 mvp = viewProj * cmd.transform;
            shader->setUniform("MVP", mvp);
            
            switch (cmd.blendMode)
            {
                case BlendMode::Opaque:
                    glEnable(GL_CULL_FACE);
                    glDisable(GL_BLEND);
                    glDisable(GL_POLYGON_OFFSET_FILL);
                    break;
                    
                case BlendMode::Mask:
                    glDisable(GL_CULL_FACE);
                    glDisable(GL_BLEND);
                    glDisable(GL_POLYGON_OFFSET_FILL);
//                    glEnable(GL_POLYGON_OFFSET_FILL);
//                    glPolygonOffset(cmd.lightmapTexture == 0 ? -16 : -8, 1);
                    break;
                    
                case BlendMode::Add:
                    glDisable(GL_CULL_FACE);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(cmd.lightmapTexture == 0 ? -16 : -8, 1);
                    break;
                    
                case BlendMode::Transparent:
                    glDisable(GL_CULL_FACE);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(cmd.lightmapTexture == 0 ? -16 : -8, 1);
                    break;
            }
            
            
            if (lastBaseTexture != cmd.baseTexture)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, cmd.baseTexture);
                
                lastBaseTexture = cmd.baseTexture;
            }
            
            if (lastLightmapTexture != cmd.lightmapTexture)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, cmd.lightmapTexture);
                
                lastLightmapTexture = cmd.lightmapTexture;
            }
            
            if (cmd.pipeline == PipelineType::World)
            {
                float cutoff = cmd.blendMode == BlendMode::Mask ? 0.5 : 0.0;
                shader->setUniform("u_AlphaCutoff", cutoff);
                
                shader->setUniform("u_hasLightmap", cmd.lightmapTexture > 0);
            }
            
            if (cmd.pipeline == PipelineType::Skinned)
            {
                shader->setUniform("u_ambient", cmd.lightProbe.ambient);
                shader->setUniform("u_color", cmd.lightProbe.color);
                shader->setUniform("u_dir", cmd.lightProbe.dir);
                
                // Model matrix uses for rotate normals
                shader->setUniform("uModel", cmd.transform);
                
                shader->setUniform("uBoneTransforms", *cmd.bones);
            }
            
            
            if (lastVAO != cmd.vao)
            {
                glBindVertexArray(cmd.vao);
                lastVAO = cmd.vao;
            }
            
            glDrawElements(GL_TRIANGLES, cmd.count, GL_UNSIGNED_INT, (void *)cmd.bufferOffset);
        }
    }
    
    void commit(Camera* camera)
    {
        if (camera == nullptr) return;
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        glCullFace(GL_FRONT);
        
        glClearColor(0.1, 0.1, 0.1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4x4 viewProj = camera->projection * camera->view;
        
        glDepthMask(GL_TRUE);
        draw(viewProj, opaquePass);
        opaquePass.clear();
        
        glDepthMask(GL_FALSE);
        draw(viewProj, transparentPass);
        transparentPass.clear();
        
        glDepthMask(GL_TRUE);
    }
}
