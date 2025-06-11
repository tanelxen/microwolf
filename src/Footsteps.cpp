//
//  Footsteps.cpp
//  wolf
//
//  Created by Fedor Artemenkov on 10.06.25.
//

#include "Footsteps.h"
#include "PlayerMovement.h"
#include "miniaudio.h"

static float step_timer = 0.0f;

static int prev_step_id = -1;
static bool is_right_step = true;
static ma_sound g_default_sound_steps[4];
static ma_sound g_wood_sound_steps[4];
static ma_sound g_snow_sound_steps[4];
static ma_sound g_clank_sound_steps[4];

static ma_engine g_engine;

void Footsteps::init()
{
    ma_engine_init(NULL, &g_engine);
    
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/step1.wav", 0, NULL, NULL, &g_default_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/step2.wav", 0, NULL, NULL, &g_default_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/step3.wav", 0, NULL, NULL, &g_default_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/step4.wav", 0, NULL, NULL, &g_default_sound_steps[3]);
    
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/wood1.wav", 0, NULL, NULL, &g_wood_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/wood2.wav", 0, NULL, NULL, &g_wood_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/wood3.wav", 0, NULL, NULL, &g_wood_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/wood4.wav", 0, NULL, NULL, &g_wood_sound_steps[3]);
    
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/snow1.wav", 0, NULL, NULL, &g_snow_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/snow2.wav", 0, NULL, NULL, &g_snow_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/snow3.wav", 0, NULL, NULL, &g_snow_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/snow4.wav", 0, NULL, NULL, &g_snow_sound_steps[3]);
    
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/clank1.wav", 0, NULL, NULL, &g_clank_sound_steps[0]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/clank2.wav", 0, NULL, NULL, &g_clank_sound_steps[1]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/clank3.wav", 0, NULL, NULL, &g_clank_sound_steps[2]);
    ma_sound_init_from_file(&g_engine, "assets/wolf/sounds/player/footsteps/clank4.wav", 0, NULL, NULL, &g_clank_sound_steps[3]);
    
    for (int i = 0; i < 4; ++i)
    {
        ma_sound_set_spatialization_enabled(&g_default_sound_steps[i], false);
        ma_sound_set_spatialization_enabled(&g_wood_sound_steps[i], false);
        ma_sound_set_spatialization_enabled(&g_snow_sound_steps[i], false);
        ma_sound_set_spatialization_enabled(&g_clank_sound_steps[i], false);
    }
}

#define SURF_WOOD 0x40000
#define SURF_SNOW 0x400000
#define SURF_METAL 0x1000

void Footsteps::update(float dt, struct PlayerMovement *movement)
{
    if (movement == nullptr) return;
    
    if (!movement->isWalk())
    {
        step_timer = 0.0f;
        is_right_step = true;
        return;
    }
    
    if (step_timer == 0.0f)
    {
        int step_id = rand() % 4;
        
        if (step_id == prev_step_id)
        {
            step_id = (step_id + 1) % 4;
        }
        
        prev_step_id = step_id;
        
        int surfaceFlags = movement->getSurfaceFlags();
        
        float LO = 0.9f;
        float HI = 1.1f;
        float pitch = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
        
        float pan = is_right_step ? 0.1 : -0.1;
        
        if (surfaceFlags & SURF_WOOD)
        {
            ma_sound& sound = g_wood_sound_steps[step_id];
            
            ma_sound_set_pitch(&sound, pitch);
            ma_sound_set_pan(&sound, pan);
            ma_sound_start(&sound);
        }
        else if (surfaceFlags & SURF_SNOW)
        {
            ma_sound& sound = g_snow_sound_steps[step_id];
            
            ma_sound_set_pitch(&sound, pitch);
            ma_sound_set_pan(&sound, pan);
            ma_sound_start(&sound);
        }
        else if (surfaceFlags & SURF_METAL)
        {
            ma_sound& sound = g_clank_sound_steps[step_id];
            
            ma_sound_set_pitch(&sound, pitch);
            ma_sound_set_pan(&sound, pan);
            ma_sound_start(&sound);
        }
        else
        {
            ma_sound& sound = g_default_sound_steps[step_id];
            
            ma_sound_set_pitch(&sound, pitch);
            ma_sound_set_pan(&sound, pan);
            ma_sound_start(&sound);
        }
        
        is_right_step = !is_right_step;
    }
    
    step_timer += dt;
    
    if (step_timer > 0.35f)
    {
        step_timer = 0.0f;
    }
}
