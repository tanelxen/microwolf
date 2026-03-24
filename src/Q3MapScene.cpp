//
//  Q3MapScene.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.10.24.
//

#include "Q3MapScene.h"

#include "KeyValueCollection.h"
#include "Q3BSPAsset.h"
#include "Camera.h"
#include "Player.h"
#include "PlayerMovement.h"

#include <glad/glad.h>

#include "miniaudio.h"

#include "GoldSrcModel.h"
#include "Q3LightGrid.h"

#include "Monster.h"

#include "Input.h"

#include "DebugRenderer.h"
#include "Cube.h"

#include "RenderDevice.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define degrees(rad) ((rad) * (180.0f / M_PI))
#define radians(deg) ((deg) * (M_PI / 180.0f))

static ma_engine g_engine;

static WiredCube cube;

tBSPVisData m_clusters;



struct SoundEntity
{
    ma_sound sound;
    
    std::string filename;
    glm::vec3 origin;
    
    int cluster;
    int area;
    
    bool isVisible = false;
};

static std::vector<SoundEntity> g_ambients;

std::unordered_map<std::string, GoldSrcModel> studio_cache;

std::unique_ptr<GoldSrcModelInstance> makeModelInstance(const std::string& filename);

Q3MapScene::Q3MapScene(Camera *camera) : m_pCamera(camera)
{
    m_pLightGrid = std::make_unique<Q3LightGrid>();
    
    ma_engine_init(NULL, &g_engine);
    
    loadMap("assets/wolf/maps/escape2.bsp");
    
    cube.init();
    
    for (auto& entity : g_ambients)
    {
        ma_sound_init_from_file(&g_engine, entity.filename.c_str(), 0, NULL, NULL, &entity.sound);
        
        ma_sound_set_spatialization_enabled(&entity.sound, true);
        ma_sound_set_positioning(&entity.sound, ma_positioning_absolute);
        ma_sound_set_attenuation_model(&entity.sound, ma_attenuation_model_linear);
        
        ma_sound_set_rolloff(&entity.sound, 1.0f);
        ma_sound_set_min_distance(&entity.sound, 80.0f);
        ma_sound_set_max_distance(&entity.sound, 1250.0f);
        
        ma_sound_set_position(&entity.sound, entity.origin.x, entity.origin.y, entity.origin.z);
        ma_sound_set_looping(&entity.sound, true);
        
        m_collision.findClusterArea(entity.origin, entity.cluster, entity.area);
    }
}

Q3MapScene::~Q3MapScene() = default;

void Q3MapScene::loadMap(const std::string &filename)
{
    Q3BSPAsset bsp;

    if (!bsp.initFromFile(filename.c_str())) {
        return;
    }
    
    m_collision.initFromBsp(&bsp);
    
    m_pMovement = std::make_unique<PlayerMovement>(&m_collision);
    
    m_pPlayer = std::make_unique<Player>(m_pMovement.get());
    
    KeyValueCollection entities;
    entities.initFromString(bsp.m_entities);
    
    auto info_player_start = entities.getAllWithKeyValue("classname", "info_player_start");
    auto ai_soldier = entities.getAllWithKeyValue("classname", "ai_soldier");
    
    auto spawnPoints = info_player_start;
    spawnPoints.insert(spawnPoints.end(), ai_soldier.begin(), ai_soldier.end());
    
    for (int i = 0; i < spawnPoints.size(); ++i)
    {
        auto spawnPoint = spawnPoints[i];
        
        float yaw = 0;
        glm::vec3 position = {0, 0, 0};
        
        int angle = 0;
        
        if (spawnPoint.getIntValue("angle", angle))
        {
            yaw = radians(angle);
        }
        
        glm::vec3 origin = {0, 0, 0};
        
        if (spawnPoint.getVec3Value("origin", origin))
        {
            printf("entity_%i: origin = (%.0f, %.0f, %.0f)\n", i, origin.x, origin.y, origin.z);
            position = origin;
        }
        
        if (i == 0)
        {
            m_pPlayer->m_pModelInstance = makeModelInstance("assets/models/v_9mmhandgun.mdl");
            m_pPlayer->position = position + glm::vec3{0, 0, 0.25};
            m_pPlayer->yaw = yaw;
            m_pPlayer->pitch = 0;
            
            m_pPlayer->m_pModelInstance->animator.setSeqIndex(2);
        }
        else
        {
            auto& monster = m_monsters.emplace_back();
            monster.m_pModelInstance = makeModelInstance("assets/models/barney.mdl");
            monster.position = position;
            monster.yaw = yaw;
            
            int seqIndex = (int) (m_monsters.size() - 1) % monster.m_pModelInstance->animator.getNumSeq();
            monster.m_pModelInstance->animator.setSeqIndex(seqIndex);
        }
    }
    
    auto target_speaker = entities.getAllWithKeyValue("classname", "target_speaker");
    
    for (int i = 0; i < target_speaker.size(); ++i)
    {
        auto& speaker = target_speaker[i];
        
        int spawnflags = -1;
        speaker.getIntValue("spawnflags", spawnflags);
        
        if (spawnflags != 1) continue;
        
        glm::vec3 origin = {0, 0, 0};
        if (speaker.getVec3Value("origin", origin) == false) continue;
        
        if (speaker.properties.contains("noise") == false) continue;
        
        SoundEntity& entity = g_ambients.emplace_back();
        
        entity.filename = "assets/wolf/" + speaker.properties["noise"];
        entity.origin = origin;
    }
    
    auto func_door_rotating = entities.getAllWithKeyValue("classname", "func_door_rotating");
    
    for (auto& desc : func_door_rotating)
    {
        glm::vec3 origin = {0, 0, 0};
        if (desc.getVec3Value("origin", origin) == false) continue;
        
        if (desc.properties.contains("model") == false) continue;
        
        std::string& model = desc.properties["model"]; // it looks like "*23"
        
        int id = std::stoi(model.substr(1));
        
        int faceIndex = bsp.m_models[id].faceIndex;
        int numOfFaces = bsp.m_models[id].numOfFaces;
        
        std::unordered_map<int, bool> processedVertices;
        
        for (int i = faceIndex; i < faceIndex + numOfFaces; ++i)
        {
            const tBSPFace &face = bsp.m_faces[i];
            
            for (int j = face.startIndex; j < face.startIndex + face.numOfIndices; ++j)
            {
                unsigned int index = bsp.m_indices[j] + face.startVertIndex;
                
                if (processedVertices[index]) {
                    continue;
                }
                
                tBSPVertex& vertex = bsp.m_verts[index];
                vertex.vPosition += origin;
                
                processedVertices[index] = true;
            }
        }
    }
    
    m_mesh.initFromBsp(&bsp);
    
    m_pLightGrid->init(bsp);
    
    m_clusters = bsp.m_clusters;
}

void Q3MapScene::update(float dt)
{
    if (m_pCamera == nullptr) return;
    
    m_pPlayer->update(dt);
    
    glm::vec3 camera_pos = m_pPlayer->position;
    camera_pos.z += 40;
    
    m_pCamera->setTransform(camera_pos, m_pPlayer->forward, m_pPlayer->right, m_pPlayer->up);
    
    {
        glm::vec3 start = m_pPlayer->position;
        start.z += 40;
        
        ma_engine_listener_set_position(&g_engine, 0, start.x, start.y, start.z);
        
        ma_engine_listener_set_direction(&g_engine, 0, m_pCamera->getForward().x, m_pCamera->getForward().y, m_pCamera->getForward().z);
        ma_engine_listener_set_world_up(&g_engine, 0, 0, 0, 1);
    }
    
    for (auto& monster : m_monsters)
    {
        monster.update(dt);
    }
    
    if (Input::isLeftMouseButtonClicked())
    {
        glm::vec3 start = m_pPlayer->position + glm::vec3{0, 0, 40};
        glm::vec3 end = start + m_pCamera->getForward() * 2048.0f;
        
        DebugRenderer::getInstance().addLine(start, end, glm::vec3(1), 10.0f);
        
        if (auto hitted = traceEntities(start, end))
        {
            glm::vec3 dirToPlayer = glm::normalize(m_pPlayer->position - hitted->position);
            hitted->yaw = atan2(dirToPlayer.y, dirToPlayer.x);
        }
    }
    
    DebugRenderer::getInstance().update(dt);
    
    int playerCluster, playerArea;
    m_collision.findClusterArea(m_pPlayer->position, playerCluster, playerArea);
    
    for (auto& entity : g_ambients)
    {
        int i = (playerCluster * m_clusters.bytesPerCluster) + (entity.cluster >> 3);
        byte visSet = m_clusters.pBitsets[i];
        
        bool isVisible = (visSet & (1 << (entity.cluster & 7))) != 0;
        
        if (isVisible)
        {
            isVisible = playerArea == entity.area;
        }
        
        if (entity.isVisible != isVisible)
        {
            isVisible ? ma_sound_start(&entity.sound) : ma_sound_stop(&entity.sound);
            entity.isVisible = isVisible;
        }
    }
}

void drawModels(GoldSrcModelInstance* inst, const Q3LightGrid* lightGrid)
{
    glm::mat4 model = glm::mat4(1);
    model = glm::translate(model, inst->position);
    model = glm::rotate(model, inst->yaw, glm::vec3(0, 0, 1));
    model = glm::rotate(model, inst->pitch, glm::vec3(0, 1, 0));
    
    if (inst->hasParent)
    {
        glm::mat4 parent = glm::mat4(1);
        parent = glm::translate(parent, inst->parent_position);
        parent = glm::rotate(parent, inst->parent_yaw, glm::vec3(0, 0, 1));
        parent = glm::rotate(parent, inst->parent_pitch, glm::vec3(0, 1, 0));
        
        model = parent * model;
    }
    
    glm::vec3 ambient = glm::vec3{1};
    glm::vec3 color = glm::vec3{1};
    glm::vec3 dir = glm::vec3{0};
    
    if (lightGrid != nullptr)
    {
        glm::vec3 pos = inst->parent_position + inst->position;
        
        if (inst->hasParent == false) {
            pos.z += 24;
        }
        
        lightGrid->getValue(pos, ambient, color, dir);
    }
    
    for (auto& surface : inst->m_pmodel->mesh.surfaces)
    {
        RenderCommand cmd;
        cmd.vao = inst->m_pmodel->mesh.vao;
        cmd.ibo = inst->m_pmodel->mesh.ibo;
        cmd.bufferOffset = surface.bufferOffset;
        cmd.count = surface.indicesCount;
        
        cmd.pipeline = PipelineType::Skinned;
        cmd.baseTexture = inst->m_pmodel->mesh.textures[surface.tex];
        
        cmd.transform = model;
        
        cmd.lightProbe = { .ambient = ambient, .color = color, .dir = dir };
        
        cmd.bones = &(inst->animator.transforms);
        
        cmd.passFlags = MainPass;
        
        RenderDevice::submit(cmd);
    }
}

void Q3MapScene::draw()
{
    if (m_pCamera == nullptr) return;
    
    m_mesh.renderFaces();
    
    for (auto& monster : m_monsters)
    {
        drawModels(monster.m_pModelInstance.get(), m_pLightGrid.get());
    }
    
    drawModels(m_pPlayer->m_pModelInstance.get(), m_pLightGrid.get());
    
    RenderDevice::commit(m_pCamera);
    
    glm::mat4x4 viewProj = m_pCamera->projection * m_pCamera->view;
    DebugRenderer::getInstance().draw(viewProj);
}

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/gtx/norm.hpp>

bool intersection(const glm::vec3& start, const glm::vec3& end, const glm::vec3& mins, const glm::vec3& maxs, glm::vec3& point);

Monster* Q3MapScene::traceEntities(const glm::vec3 &start, const glm::vec3 &end)
{
    HitResult result;
    m_collision.trace(result, start, end, glm::vec3(0), glm::vec3(0));
    
    float maxDist = glm::distance2(result.endpos, start);
    
    Monster* nearestMonster = nullptr;
    float nearestDist = maxDist;
    
    for (auto& monster : m_monsters)
    {
        glm::vec3 mins = monster.position + glm::vec3{-15, -15, -24};
        glm::vec3 maxs = monster.position + glm::vec3{ 15,  15,  48};
        
        glm::vec3 point;
        bool check = intersection(start, end, mins, maxs, point);
        
        if (check)
        {
            float dist = glm::distance2(point, start);
            
            if (dist < nearestDist)
            {
                nearestDist = dist;
                nearestMonster = &monster;
            }
        }
    }
    
    return nearestMonster;
}


#include <algorithm>
using std::min, std::max, std::swap;

bool intersection(const glm::vec3& start, const glm::vec3& end, const glm::vec3& mins, const glm::vec3& maxs, glm::vec3& point)
{
    glm::vec3 dir = end - start;
    
    glm::vec3 t1 = (mins - start) / dir;
    glm::vec3 t2 = (maxs - start) / dir;

    // Ensure tmin and tmax are sorted
    if (t1.x > t2.x) { swap(t1.x, t2.x); }
    if (t1.y > t2.y) { swap(t1.y, t2.y); }
    if (t1.z > t2.z) { swap(t1.z, t2.z); }

    float tmin = max(max(t1.x, t1.y), t1.z);
    float tmax = min(min(t2.x, t2.y), t2.z);

    if (tmax > tmin && tmax > 0.0)
    {
        point = start + dir * tmin;
        return true;
    }
    
    return false;
}


GoldSrcModel* makeModel(const std::string& filename)
{
    auto [it, inserted] = studio_cache.try_emplace(filename);
    
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

std::unique_ptr<GoldSrcModelInstance> makeModelInstance(const std::string& filename)
{
    auto inst = std::make_unique<GoldSrcModelInstance>();
    
    inst->m_pmodel = makeModel(filename);
    inst->animator.m_pAnimation = &(inst->m_pmodel->animation);
    
    return inst;
}
