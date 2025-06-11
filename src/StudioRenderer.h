//
//  StudioRenderer.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 19.11.24.
//

#pragma once

#include <memory>

#include "Shader.h"
#include "GoldSrcModel.h"

class Camera;
struct Q3LightGrid;

struct StudioRenderer
{
    StudioRenderer();
    
    std::unique_ptr<GoldSrcModelInstance> makeModelInstance(const std::string& filename);
    
    void queue(GoldSrcModelInstance* inst);
    void queueViewModel(GoldSrcModelInstance* inst);
    
    void drawRegular(const Camera* camera, const Q3LightGrid* lightGrid);
    void drawViewModels(const Camera* camera, const Q3LightGrid* lightGrid);
    
private:
    Shader m_shader;
    
    std::unordered_map<std::string, GoldSrcModel> m_cache;
    std::vector<GoldSrcModelInstance*> m_renderQueue;
    std::vector<GoldSrcModelInstance*> m_renderQueueView;
    
    // May use for pre-caching
    GoldSrcModel* makeModel(const std::string& filename);
};
