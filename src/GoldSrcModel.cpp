//
//  RenderableModel.cpp
//  hlmv
//
//  Created by Fedor Artemenkov on 17.11.24.
//

#include "GoldSrcModel.h"
#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/quaternion.hpp>

//#pragma warning( disable : 4244 ) // conversion from 'double ' to 'float ', possible loss of data
//#pragma warning( disable : 4305 ) // truncation from 'const double ' to 'float '

#define VERT_POSITION_LOC 0
#define VERT_NORMAL_LOC 1
#define VERT_DIFFUSE_TEX_COORD_LOC 2
#define VERT_BONE_INDEX_LOC 3


void GoldSrcMesh::init(const GoldSrc::Model& model)
{
    uploadTextures(model.textures);
    uploadMeshes(model.meshes);
}

void GoldSrcMesh::draw()
{
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    
    for (auto& surface : surfaces)
    {
        unsigned int texId = textures[surface.tex];
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texId);
        
        glDrawElements(GL_TRIANGLES, surface.indicesCount, GL_UNSIGNED_INT, (void*)surface.bufferOffset);
    }
}

void GoldSrcMesh::uploadTextures(const std::vector<GoldSrc::Texture> &textures)
{
    this->textures.resize(textures.size());
    
    for (int i = 0; i < textures.size(); ++i)
    {
        const GoldSrc::Texture& item = textures[i];
        
        unsigned int id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, item.width, item.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, item.data.data());
        glGenerateMipmap(GL_TEXTURE_2D);
        
        this->textures[i] = id;
    }
}

void GoldSrcMesh::uploadMeshes(const std::vector<GoldSrc::Mesh> &meshes)
{
    std::vector<GoldSrc::MeshVertex> vertices;
    std::vector<unsigned int> indices;
    
    for (auto& mesh : meshes)
    {
        RenderableSurface& surface = surfaces.emplace_back();
        surface.tex = mesh.textureIndex;
        surface.bufferOffset = (int) indices.size() * sizeof(unsigned int);
        surface.indicesCount = (int) mesh.indexBuffer.size();
        
        int indicesOffset = (int) vertices.size();
        
        for (int i = 0; i < mesh.indexBuffer.size(); ++i)
        {
            indices.push_back(indicesOffset + mesh.indexBuffer[i]);
        }
        
        vertices.insert(vertices.end(), mesh.vertexBuffer.begin(), mesh.vertexBuffer.end());
    }
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GoldSrc::MeshVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glEnableVertexAttribArray(VERT_POSITION_LOC);
    glVertexAttribPointer(VERT_POSITION_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(GoldSrc::MeshVertex), (void*)offsetof(GoldSrc::MeshVertex, position));
    
    glEnableVertexAttribArray(VERT_NORMAL_LOC);
    glVertexAttribPointer(VERT_NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(GoldSrc::MeshVertex), (void*)offsetof(GoldSrc::MeshVertex, normal));
    
    glEnableVertexAttribArray(VERT_DIFFUSE_TEX_COORD_LOC);
    glVertexAttribPointer(VERT_DIFFUSE_TEX_COORD_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(GoldSrc::MeshVertex), (void*)offsetof(GoldSrc::MeshVertex, texCoord));
    
    glEnableVertexAttribArray(VERT_BONE_INDEX_LOC);
    glVertexAttribIPointer(VERT_BONE_INDEX_LOC, 1, GL_INT, sizeof(GoldSrc::MeshVertex), (void*)offsetof(GoldSrc::MeshVertex, boneIndex));
    
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);
}



void GoldSrcAnimator::setSeqIndex(int index)
{
    cur_seq_index = index;
    cur_frame_time = 0;
    cur_frame = 0;
}

int GoldSrcAnimator::getSeqIndex() const
{
    return cur_seq_index;
}

int GoldSrcAnimator::getNumSeq() const
{
    if (m_pAnimation == nullptr) return 0;
    return (int) m_pAnimation->sequences.size();
}

void GoldSrcAnimator::update(float dt)
{
    if (m_pAnimation == nullptr) return;
    if (cur_seq_index >= m_pAnimation->sequences.size()) return;
    
    const GoldSrc::Sequence& seq = m_pAnimation->sequences[cur_seq_index];
    
    cur_anim_duration = (float)seq.frames.size() / seq.fps;
    
    updatePose();
    
    cur_frame_time += dt;
    
    if (cur_frame_time >= cur_anim_duration)
    {
        cur_frame_time = 0;
    }
    
    cur_frame = (float)seq.frames.size() * (cur_frame_time / cur_anim_duration);
}

void GoldSrcAnimator::updatePose()
{
    if (m_pAnimation == nullptr) return;
    
    if (transforms.size() < m_pAnimation->bones.size())
    {
        transforms.resize(m_pAnimation->bones.size());
    }
    
    const GoldSrc::Sequence& seq = m_pAnimation->sequences[cur_seq_index];
    
    int currIndex = int(cur_frame);
    int nextIndex = (currIndex + 1) % seq.frames.size();
    
    float factor = cur_frame - floor(cur_frame);
    
    const GoldSrc::Frame& curr = seq.frames[currIndex];
    const GoldSrc::Frame& next = seq.frames[nextIndex];
    
    for (int i = 0; i < m_pAnimation->bones.size(); ++i)
    {
        const glm::quat& currRotation = curr.rotationPerBone[i];
        const glm::quat& nextRotation = next.rotationPerBone[i];
        
        const glm::vec3& currPosition = curr.positionPerBone[i];
        const glm::vec3& nextPosition = next.positionPerBone[i];
        
        glm::quat rotation = currRotation * (1.0f - factor) + nextRotation * factor;
        glm::vec3 position = currPosition * (1.0f - factor) + nextPosition * factor;
        
        glm::mat4& transform = transforms[i];
        transform = glm::toMat4(rotation);
        
        transform[3][0] = position[0];
        transform[3][1] = position[1];
        transform[3][2] = position[2];
        
        if (m_pAnimation->bones[i] != -1)
        {
            transform = transforms[m_pAnimation->bones[i]] * transform;
        }
    }
}

glm::vec3 GoldSrcAnimator::getMinBounds() const
{
    return m_pAnimation->sequences[cur_seq_index].bbmin;
}

glm::vec3 GoldSrcAnimator::getMaxBounds() const
{
    return m_pAnimation->sequences[cur_seq_index].bbmax;
}

