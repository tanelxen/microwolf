//
//  Q3Bezier.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 24.11.24.
//

#include "Q3BSPTypes.h"

tBSPVertex mix(const tBSPVertex& x, const tBSPVertex& y, float t)
{
    tBSPVertex v;
    
    v.vPosition = glm::mix(x.vPosition, y.vPosition, t);
    v.vTextureCoord = glm::mix(x.vTextureCoord, y.vTextureCoord, t);
    v.vLightmapCoord = glm::mix(x.vLightmapCoord, y.vLightmapCoord, t);
    
    return v;
}

tBSPVertex quadraticBezier(const tBSPVertex& A, const tBSPVertex& B, const tBSPVertex& C, float t)
{
    tBSPVertex D = mix(A, B, t);
    tBSPVertex E = mix(B, C, t);
    
    return mix(D, E, t);
}

tBSPVertex quadraticBezierSurface(const tBSPVertex* patch, int stride, int ix, float stepX, int iy, float stepY, tBSPVertex* cache, uint64_t& cacheFill)
{
    cache += ix * 3;
    const uint64_t cacheFillMask = (1ULL << ix);
    
    if ((cacheFill & cacheFillMask) == 0)
    {
        // cache these values instead of calculating them all the time
        cacheFill |= cacheFillMask;
        for (int i = 0; i < 3; i++, patch += stride)
        {
            cache[i] = quadraticBezier(patch[0], patch[1], patch[2], ix * stepX);
        }
    }
    
    return quadraticBezier(cache[0], cache[1], cache[2], iy * stepY);
}

int getNumBezierPatches(const std::vector<tBSPFace>& faces)
{
    int numPatches = 0;
    
    for (const auto& face : faces)
    {
        if (face.type == 2)
        {
            numPatches += (face.size[0] - 1) * (face.size[1] - 1) / 4;
        }
    }
    
    return numPatches;
}

void addPatchTriangles(const tBSPVertex* patch, int stride, int numVertexPerSide, int baseVertexIndex, std::vector<tBSPVertex>& outVertices, std::vector<int>& outIndices)
{
    baseVertexIndex = outVertices.size() - baseVertexIndex;
    
    int numVerticesX = numVertexPerSide;
    int numVerticesY = numVertexPerSide;
    
    // add vertices
    uint64_t cacheFill = 0u;
    std::vector<tBSPVertex> cache(numVerticesX * 3);
    
    float sx = 1.f / (numVerticesX - 1);
    float sy = 1.f / (numVerticesY - 1);
    
    for (int j = 0; j < numVerticesY; j++)
    {
        for (int i = 0; i < numVerticesX; i++)
        {
            tBSPVertex newVert = quadraticBezierSurface(patch, stride, i, sx, j, sy, &cache[0], cacheFill);
            outVertices.push_back(newVert);
        }
    }
    
    // add indices
    for (int j = 0; j < numVerticesY - 1; j++)
    {
        for (unsigned i = 0; i < numVerticesX - 1; i++)
        {
            // for each cell in the patch, triangulate
            int ix0 = baseVertexIndex + i + j * numVertexPerSide;
            int ix1 = ix0 + 1;
            int ix2 = ix0 + numVertexPerSide;
            int ix3 = ix2 + 1;
            
            // first triangle
            outIndices.push_back(ix0);
            outIndices.push_back(ix3);
            outIndices.push_back(ix1);
            
            // second triangle
            outIndices.push_back(ix0);
            outIndices.push_back(ix2);
            outIndices.push_back(ix3);
        }
    }
}

void tesselateBezierPatches(std::vector<tBSPFace>& faces, std::vector<tBSPVertex>& verts, std::vector<int>& indices, int level)
{
    // reserve memory
    int numPatches = getNumBezierPatches(faces);
    verts.reserve(verts.size() + numPatches * level * level);
    indices.reserve(indices.size() + numPatches * (level - 1) * (level - 1) / 4 * 6);
    
    for (auto& face : faces)
    {
        if (face.type != 2) continue;
        
        int baseVertex = verts.size();
        int baseMeshVertex = indices.size();
        
        const int stride = face.size[0];
        const int iSize = (face.size[0] - 1) / 2;
        const int jSize = (face.size[1] - 1) / 2;
        
        for (int i = 0; i < iSize; i++)
        {
            for (int j = 0; j < jSize; j++)
            {
                // for each patch of 3x3 control points
                int basePatchIndex = 2 * (i + j * stride); // relative to f.mVertex
                auto patch = &verts[face.startVertIndex + basePatchIndex];
                addPatchTriangles(patch, stride, level, baseVertex, verts, indices);
            }
        }
        
        face.startVertIndex = baseVertex;
        face.startIndex = baseMeshVertex;
        face.numOfVerts = verts.size() - baseVertex;
        face.numOfIndices = indices.size() - baseMeshVertex;
    }
    
    verts.shrink_to_fit();
    indices.shrink_to_fit();
}
