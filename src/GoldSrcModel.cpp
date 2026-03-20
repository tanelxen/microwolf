//
//  RenderableModel.cpp
//  hlmv
//
//  Created by Fedor Artemenkov on 17.11.24.
//

#include "GoldSrcModel.h"

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/quaternion.hpp>

#include "RenderDevice.h"

void GoldSrcMesh::init(const GoldSrc::Model& model)
{
    uploadTextures(model.textures);
    uploadMeshes(model.meshes);
}

void GoldSrcMesh::uploadTextures(const std::vector<GoldSrc::Texture> &textures)
{
    this->textures.resize(textures.size());
    
    for (int i = 0; i < textures.size(); ++i)
    {
        const GoldSrc::Texture& item = textures[i];
        this->textures[i] = RenderDevice::makeTexture(item.width, item.height, true, true, item.data.data());
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
    
    vbo = RenderDevice::makeVertexBuffer(sizeof(GoldSrc::MeshVertex) * vertices.size(), vertices.data());
    ibo = RenderDevice::makeIndexBuffer(sizeof(int) * indices.size(), indices.data());
    
    
    VertexLayout layout;
    layout.add<float>(3);
    layout.add<float>(3);
    layout.add<float>(2);
    layout.add<unsigned int>(1);
    
    vao = RenderDevice::makeVertexArray(vbo, layout);
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

