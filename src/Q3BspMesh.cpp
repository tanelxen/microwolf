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
        BlendMode blendMode = BlendMode::Opaque;
        
        s_q3shaders.getBaseTextureName(g_bsp->m_textures[i].strName, mapParam, blendMode);
        
        Image image = loadImage(mapParam);
        
        if (image.data)
        {
            bool hasAlpha = image.num_channels == 4;
            
            m_textures[i].baseTextureId = RenderDevice::makeTexture(image.width, image.height, hasAlpha, true, image.data);
            m_textures[i].blendMode = blendMode;
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
}

void Q3BspMesh::renderFaces()
{
    for (auto surface : lm_surfaces)
    {
        auto& material = m_textures[surface.textureNum];
        
        RenderCommand cmd = {
            .vao = vao,
            .bufferOffset = surface.bufferOffset,
            .count = surface.numVerts,
    
            .pipeline = PipelineType::World,
            .baseTexture = material.baseTextureId,
            .lightmapTexture = m_lightmap,
            .blendMode = material.blendMode,
            
            .passFlags = MainPass
        };
        
        RenderDevice::submit(cmd);
    }
    
    for (auto surface : vl_surfaces)
    {
        auto& material = m_textures[surface.textureNum];
        
        RenderCommand cmd = {
            .vao = vao,
            .bufferOffset = surface.bufferOffset,
            .count = surface.numVerts,
            
            .pipeline = PipelineType::World,
            .baseTexture = material.baseTextureId,
            .blendMode = material.blendMode,
            
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
        
        auto& surface = lm_surfaces.emplace_back();
        surface.bufferOffset = (uint32_t)(indices.size() * sizeof(uint32_t));
        surface.numVerts = (uint32_t)pair.second.size();
        surface.textureNum = texture;
        
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
        
        int flags = g_bsp->m_textures[texture].flags;
        
        if (flags & SURFACE_SKY) continue;
        
        auto& surface = vl_surfaces.emplace_back();
        surface.bufferOffset = (uint32_t)(indices.size() * sizeof(uint32_t));
        surface.numVerts = (uint32_t)pair.second.size();
        surface.textureNum = texture;
        
        indices.insert(indices.end(), pair.second.begin(), pair.second.end());
    }
}

