//
//  BspScene.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 27.10.24.
//

#include <glad/glad.h>

#include "../vendor/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../vendor/stb_image.h"

#include <unordered_map>
#include <iostream>
#include <string>
#include <vector>

#include "Q3BspMesh.h"
#include "Q3BSPAsset.h"
#include "Camera.h"
#include "Shader.h"
#include "TextureAtlas.h"


#define FACE_POLYGON 1
#define FACE_PATCH 2
#define FACE_MESH 3
#define FACE_FX 4

static Q3BSPAsset* g_bsp = nullptr;
static Shader lm_shader;
static Shader vl_shader;

static void adjastLightmapCoords(Q3BSPAsset* bsp, const TextureAtlas& atlas);
static GLuint generateLightmap(const TextureAtlas& atlas);

static bool textures_alpha[MAX_TEXTURES];

void Q3BspMesh::initFromBsp(Q3BSPAsset* bsp)
{
    g_bsp = bsp;
    
    TextureAtlas atlas;
    atlas.initFromQ3Lightmaps(bsp->m_lightmaps);
//    g_atlas.saveToPng("lightmap.png");
    
    adjastLightmapCoords(bsp, atlas);
    m_lightmap = generateLightmap(atlas);
    
    GenerateTexture();
    initBuffers();
    
    g_bsp = nullptr;
}

void adjastLightmapCoords(Q3BSPAsset* bsp, const TextureAtlas& atlas)
{
    std::unordered_map<int, bool> processedVertices;
    
    for (int i = 0; i < bsp->m_faces.size(); ++i)
    {
        const tBSPFace &face = bsp->m_faces[i];
        
        if (face.type == FACE_FX) continue;
        if (face.lightmapID == -1) continue;
        
        const TextureTile &tile = atlas.tiles[face.lightmapID];
        
        for (int j = face.startIndex; j < face.startIndex + face.numOfIndices; ++j)
        {
            unsigned int index = bsp->m_indices[j] + face.startVertIndex;
            
            if (processedVertices[index]) {
                continue;
            }
            
            tBSPVertex& vertex = bsp->m_verts[index];
            
            vertex.vLightmapCoord.x = (vertex.vLightmapCoord.x * tile.width + tile.x) / atlas.width;
            vertex.vLightmapCoord.y = (vertex.vLightmapCoord.y * tile.height + tile.y) / atlas.height;
            
            processedVertices[index] = true;
        }
    }
}

GLuint generateLightmap(const TextureAtlas& atlas)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, atlas.width, atlas.height, 0, GL_RGB, GL_UNSIGNED_BYTE, atlas.buffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return id;
}

void Q3BspMesh::GenerateTexture()
{
    int width, height;
    int num_channels = 3;

    GLuint missing_id;
    glGenTextures(1, &missing_id); // generate missing texture

    unsigned char* image = stbi_load("assets/_engine/missing.png", &width, &height, &num_channels, 3);

    glBindTexture(GL_TEXTURE_2D, missing_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image);

    std::vector<GLuint> missing_tex_id;

    for (int i = 0; i < g_bsp->m_textures.size(); i++)
    {
        std::string path = "assets/wolf/";
        path.append(g_bsp->m_textures[i].strName);
        
        unsigned char* image = nullptr;
        
        std::string jpgPath = path + ".jpg";
        image = stbi_load(jpgPath.c_str(), &width, &height, &num_channels, 3);
        
        if (image == nullptr)
        {
            std::string tgaPath = path + ".tga";
            image = stbi_load(tgaPath.c_str(), &width, &height, &num_channels, 4);
        }

        if (image)
        {
            GLuint textureID;
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            
            bool hasAlpha = num_channels == 4;

            GLenum internalformat = hasAlpha ? GL_SRGB_ALPHA : GL_SRGB;
            GLenum format = hasAlpha ? GL_RGBA : GL_RGB;
            
            glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, image);
            glGenerateMipmap(GL_TEXTURE_2D);

            m_textures[i] = textureID;
            textures_alpha[i] = hasAlpha;
        }
        else
        {
            std::cout << "Can't find: " << path << std::endl;
            m_textures[i] = missing_id;
        }
        
        stbi_image_free(image);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

#define VERT_POSITION_LOC 0
#define VERT_DIFFUSE_TEX_COORD_LOC 1
#define VERT_LIGHTMAP_TEX_COORD_LOC 2
#define VERT_COLOR_LOC 3

void Q3BspMesh::initBuffers()
{
    makeLightmappedIndices();
    makeVertexlitIndices();
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tBSPVertex) * g_bsp->m_verts.size(), g_bsp->m_verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERT_POSITION_LOC);
    glVertexAttribPointer(VERT_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(tBSPVertex), (void*)offsetof(tBSPVertex, vPosition));

    glEnableVertexAttribArray(VERT_DIFFUSE_TEX_COORD_LOC);
    glVertexAttribPointer(VERT_DIFFUSE_TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(tBSPVertex), (void*)offsetof(tBSPVertex, vTextureCoord));

    glEnableVertexAttribArray(VERT_LIGHTMAP_TEX_COORD_LOC);
    glVertexAttribPointer(VERT_LIGHTMAP_TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(tBSPVertex), (void*)offsetof(tBSPVertex, vLightmapCoord));
    
    glEnableVertexAttribArray(VERT_COLOR_LOC);
    glVertexAttribPointer(VERT_COLOR_LOC, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(tBSPVertex), (void*)offsetof(tBSPVertex, color));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    lm_shader.init("assets/shaders/q3bsp_lightmapped.glsl");
    lm_shader.bind();
    glUniform1i(glGetUniformLocation(lm_shader.program, "s_bspTexture"), 0);
    glUniform1i(glGetUniformLocation(lm_shader.program, "s_bspLightmap"), 1);
    lm_shader.unbind();
    
    vl_shader.init("assets/shaders/q3bsp_vtxlit.glsl");
    vl_shader.bind();
    glUniform1i(glGetUniformLocation(vl_shader.program, "s_bspTexture"), 0);
    vl_shader.unbind();
}

void Q3BspMesh::renderFaces(glm::mat4x4& mvp)
{
    lm_shader.bind();
    lm_shader.setUniform("MVP", mvp);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    
    // ==================== OPAQUE ===============
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    glDisable(GL_BLEND);
    
    lm_shader.bind();
    lm_shader.setUniform("MVP", mvp);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_lightmap);
    
    for (auto surface : lm_surfaces_opaque)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surface.texId);
        
        glDrawElements(GL_TRIANGLES, surface.numVerts, GL_UNSIGNED_INT, (void *)surface.bufferOffset);
    }
    
    vl_shader.bind();
    vl_shader.setUniform("MVP", mvp);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    for (auto surface : vl_surfaces_opaque)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surface.texId);
        
        glDrawElements(GL_TRIANGLES, surface.numVerts, GL_UNSIGNED_INT, (void *)surface.bufferOffset);
    }
    
    // ==================== TRANSLUCENT ===============
    glDisable(GL_CULL_FACE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_POLYGON_OFFSET_FILL);
    
    glPolygonOffset(-8, 1);
    
    lm_shader.bind();
    lm_shader.setUniform("MVP", mvp);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_lightmap);
    
    for (auto surface : lm_surfaces_alpha)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surface.texId);
        
        glDrawElements(GL_TRIANGLES, surface.numVerts, GL_UNSIGNED_INT, (void *)surface.bufferOffset);
    }
    
    glPolygonOffset(-16, 1);
    
    vl_shader.bind();
    vl_shader.setUniform("MVP", mvp);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    for (auto surface : vl_surfaces_alpha)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, surface.texId);
        
        glDrawElements(GL_TRIANGLES, surface.numVerts, GL_UNSIGNED_INT, (void *)surface.bufferOffset);
    }
    
    glDisable(GL_POLYGON_OFFSET_FILL);
}

void Q3BspMesh::makeLightmappedIndices()
{
    std::unordered_map<int, std::vector<int>> indicesByTexture;
    
    for (int i = 0; i < g_bsp->m_faces.size(); ++i)
    {
        const tBSPFace &face = g_bsp->m_faces[i];
        
        if (face.type == FACE_FX) continue;
        if (face.lightmapID == -1) continue;
        
        for (int j = face.startIndex; j < face.startIndex + face.numOfIndices; ++j)
        {
            unsigned int index = g_bsp->m_indices[j] + face.startVertIndex;
            indicesByTexture[face.textureID].push_back(index);
        }
    }
    
    for (auto pair : indicesByTexture)
    {
        int texture = pair.first;
        auto& array = textures_alpha[texture] ? lm_surfaces_alpha : lm_surfaces_opaque;
        
        auto& surface = array.emplace_back();
        surface.texId = m_textures[texture];
        surface.bufferOffset = (uint32_t)(indices.size() * sizeof(uint32_t));
        surface.numVerts = (uint32_t)pair.second.size();
        
        indices.insert(indices.end(), pair.second.begin(), pair.second.end());
    }
}

#define SURFACE_SKY 0x4

void Q3BspMesh::makeVertexlitIndices()
{
    std::unordered_map<int, std::vector<int>> indicesByTexture;
    
    for (int i = 0; i < g_bsp->m_faces.size(); ++i)
    {
        const tBSPFace &face = g_bsp->m_faces[i];
        
        if (face.type == FACE_FX) continue;
        if (face.lightmapID != -1) continue;
        
        for (int j = face.startIndex; j < face.startIndex + face.numOfIndices; ++j)
        {
            unsigned int index = g_bsp->m_indices[j] + face.startVertIndex;
            indicesByTexture[face.textureID].push_back(index);
        }
    }
    
    for (auto pair : indicesByTexture)
    {
        int texture = pair.first;
        
//        const char* str = g_bsp->m_textures[texture].strName;
//        auto name = std::string(str);
        
        int flags = g_bsp->m_textures[texture].flags;
        
        if (flags & SURFACE_SKY) continue;
        
//        if (name.find("skies/sky_") != std::string::npos)
//        {
//            continue;
//        }
        
        auto& array = textures_alpha[texture] ? vl_surfaces_alpha : vl_surfaces_opaque;
        
        auto& surface = array.emplace_back();
        surface.texId = m_textures[texture];
        surface.bufferOffset = (uint32_t)(indices.size() * sizeof(uint32_t));
        surface.numVerts = (uint32_t)pair.second.size();
        
        indices.insert(indices.end(), pair.second.begin(), pair.second.end());
    }
}

