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
#include <filesystem>

#include "Q3BspMesh.h"
#include "Q3BSPAsset.h"
#include "Camera.h"
#include "Shader.h"
#include "TextureAtlas.h"
#include "RenderDevice.h"

#include "Q3Shaders.h"

#define FACE_POLYGON 1
#define FACE_PATCH 2
#define FACE_MESH 3
#define FACE_FX 4

static Q3BSPAsset* g_bsp = nullptr;
static Shader lm_shader;
static Shader vl_shader;

static void adjastLightmapCoords(Q3BSPAsset* bsp, const TextureAtlas& atlas);

static Quake3Shaders s_q3shaders;

void Q3BspMesh::initFromBsp(Q3BSPAsset* bsp)
{
    g_bsp = bsp;
    
    TextureAtlas atlas;
    atlas.initFromQ3Lightmaps(bsp->m_lightmaps);
//    g_atlas.saveToPng("lightmap.png");
    
    adjastLightmapCoords(bsp, atlas);
    m_lightmap = RenderDevice::makeTexture(atlas.width, atlas.height, false, false, atlas.buffer);
    
    s_q3shaders.initFromDir("assets/wolf/scripts/");
    
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

struct Image
{
    int width, height;
    int num_channels;
    void* data;
};

Image loadImage(std::string& map)
{
    Image image;
    
    std::filesystem::path folder("assets/wolf");
    std::filesystem::path path = folder / std::filesystem::path(map);
    
    image.data = stbi_load(path.replace_extension("jpg").c_str(), &image.width, &image.height, &image.num_channels, 3);
    
    if (image.data == nullptr)
    {
        image.data = stbi_load(path.replace_extension("tga").c_str(), &image.width, &image.height, &image.num_channels, 4);
    }
    
    if (image.data == nullptr)
    {
        std::cout << "Can't find: " << path << std::endl;
    }
    
    return image;
}

void Q3BspMesh::GenerateTexture()
{
    int width, height;
    int num_channels = 3;

    unsigned char* image = stbi_load("assets/_engine/missing.png", &width, &height, &num_channels, 3);
    GLuint missing_id = RenderDevice::makeTexture(width, height, false, true, image);
    stbi_image_free(image);

    for (int i = 0; i < g_bsp->m_textures.size(); i++)
    {
        m_textures[i].baseTextureId = missing_id;

        std::string mapParam = g_bsp->m_textures[i].strName;
        bool transparent = false;
        
        s_q3shaders.getBaseTextureName(g_bsp->m_textures[i].strName, mapParam, transparent);
        
        Image image = loadImage(mapParam);
        
        if (image.data)
        {
            bool hasAlpha = image.num_channels == 4;
            
            m_textures[i].baseTextureId = RenderDevice::makeTexture(image.width, image.height, hasAlpha, true, image.data);
            m_textures[i].isTransparent = transparent;
        }
        
        stbi_image_free(image.data);
    }
}

void Q3BspMesh::initBuffers()
{
    makeLightmappedIndices();
    makeVertexlitIndices();
    
    ibo = RenderDevice::makeIndexBuffer(sizeof(int) * indices.size(), indices.data());
    vbo = RenderDevice::makeVertexBuffer(sizeof(tBSPVertex) * g_bsp->m_verts.size(), g_bsp->m_verts.data());
    
    VertexLayout layout;
    layout.add<float>(3);
    layout.add<float>(2);
    layout.add<float>(2);
    layout.add<float>(3);
    layout.add<byte>(4);
    
    vao = RenderDevice::makeVertexArray(vbo, ibo, layout);
    
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

void Q3BspMesh::renderFaces()
{
    for (auto surface : lm_surfaces_opaque)
    {
        RenderCommand cmd = {
            .vao = vao,
            .ibo = ibo,
            .bufferOffset = surface.bufferOffset,
            .count = surface.numVerts,
            
            .shader = &lm_shader,
            .baseTexture = surface.texId,
            .lightmapTexture = m_lightmap,
            .isOpaque = true,
            
            .passFlags = MainPass
        };
        
        RenderDevice::submit(cmd);
    }
    
    for (auto surface : vl_surfaces_opaque)
    {
        RenderCommand cmd = {
            .vao = vao,
            .ibo = ibo,
            .bufferOffset = surface.bufferOffset,
            .count = surface.numVerts,
            
            .shader = &vl_shader,
            .baseTexture = surface.texId,
            .lightmapTexture = 0,
            .isOpaque = true,
            
            .passFlags = MainPass
        };
        
        RenderDevice::submit(cmd);
    }
    
    for (auto surface : lm_surfaces_alpha)
    {
        RenderCommand cmd = {
            .vao = vao,
            .ibo = ibo,
            .bufferOffset = surface.bufferOffset,
            .count = surface.numVerts,
            
            .shader = &lm_shader,
            .baseTexture = surface.texId,
            .lightmapTexture = m_lightmap,
            .isOpaque = false,
            
            .passFlags = MainPass
        };
        
        RenderDevice::submit(cmd);
    }
    
    for (auto surface : vl_surfaces_alpha)
    {
        RenderCommand cmd = {
            .vao = vao,
            .ibo = ibo,
            .bufferOffset = surface.bufferOffset,
            .count = surface.numVerts,
            
            .shader = &vl_shader,
            .baseTexture = surface.texId,
            .lightmapTexture = 0,
            .isOpaque = false,
            
            .passFlags = MainPass
        };
        
        RenderDevice::submit(cmd);
    }
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
        auto& array = m_textures[texture].isTransparent ? lm_surfaces_alpha : lm_surfaces_opaque;
        
        auto& surface = array.emplace_back();
        surface.texId = m_textures[texture].baseTextureId;
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
        
        auto& array = m_textures[texture].isTransparent ? vl_surfaces_alpha : vl_surfaces_opaque;
        
        auto& surface = array.emplace_back();
        surface.texId = m_textures[texture].baseTextureId;
        surface.bufferOffset = (uint32_t)(indices.size() * sizeof(uint32_t));
        surface.numVerts = (uint32_t)pair.second.size();
        
        indices.insert(indices.end(), pair.second.begin(), pair.second.end());
    }
}

