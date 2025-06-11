//
//  Q3BspCollision.h
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.10.24.
//

#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Q3BSPAsset;

struct HitResult
{
    glm::vec3 endpos;
    glm::vec3 normal;
    float fraction;
    
    bool startsolid;
    bool allsolid;
    
    int textureId;
    int surfaceFlags;
};

class Q3BspCollision
{
public:
    Q3BspCollision();
    ~Q3BspCollision();
    
    void initFromBsp(Q3BSPAsset* bsp);
    
    void trace(HitResult& result, const glm::vec3 &start, const glm::vec3 &end, const glm::vec3 &mins, const glm::vec3 &maxs) const;
    
    void findClusterArea(const glm::vec3 &pos, int &cluster, int &area);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
