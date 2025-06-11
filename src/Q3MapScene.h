//
//  Q3MapScene.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.10.24.
//

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Q3BspMesh.h"
#include "Q3BspCollision.h"

class Camera;

class Player;
class PlayerMovement;

struct StudioRenderer;
struct Q3LightGrid;

class Monster;

class Q3MapScene
{
public:
    Q3MapScene(Camera* camera);
    ~Q3MapScene();
    
    void loadMap(const std::string &filename);
    void update(float dt);
    void draw();
    
private:
    Camera* m_pCamera;
    
    std::unique_ptr<Player> m_pPlayer;
    std::unique_ptr<PlayerMovement> m_pMovement;
    
    std::vector<Monster> m_monsters;
    
    Q3BspMesh m_mesh;
    Q3BspCollision m_collision;
    
    std::unique_ptr<StudioRenderer> studio;
    std::unique_ptr<Q3LightGrid> m_pLightGrid;
    
    Monster* traceEntities(const glm::vec3 &start, const glm::vec3 &end);
};
