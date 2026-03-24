//
//  BspScene.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 27.10.24.
//

#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "RenderDevice.h"

#define MAX_TEXTURES 1000


struct Material
{
    unsigned int baseTextureId;
    BlendMode blendMode = BlendMode::Opaque;
};

struct Surface
{
    unsigned int textureNum;

    unsigned int bufferOffset; // offset in bytes
    unsigned int numVerts;
};

class Q3BSPAsset;

class Q3BspMesh
{
public:
    void initFromBsp(Q3BSPAsset* bsp);
    void renderFaces();
    
private:
    void initBuffers();
    void GenerateTexture();
    
    void makeLightmappedIndices();
    void makeVertexlitIndices();
    
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    
    Material m_textures[MAX_TEXTURES];
    unsigned int m_lightmap;

    unsigned int missing_id;
    
    std::vector<unsigned int> indices;
    
    std::vector<Surface> lm_surfaces;
    std::vector<Surface> vl_surfaces;
};
