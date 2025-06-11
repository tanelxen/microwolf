//
//  Input.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.11.24.
//

#pragma once

#define KEY_SPACE 32
#define KEY_W 87
#define KEY_S 83
#define KEY_A 65
#define KEY_D 68

struct Input
{
    static bool isKeyPressed(int keyNum);
    static bool isKeyClicked(int keyNum);
    
    static bool isLeftMouseButtonPressed();
    static bool isRightMouseButtonPressed();
    
    static bool isLeftMouseButtonClicked();
    static bool isRightMouseButtonClicked();
    
    static float getMouseOffsetX();
    static float getMouseOffsetY();
    
    static float getMousePositionX();
    static float getMousePositionY();
    
private:
    static Input& getInstance()
    {
        static Input instance;
        return instance;
    }
    
    // Only Application allows to update
    friend struct Application;
    
    bool keys[256];
    bool prev_keys[256];
    
    float m_mouseX = 0;
    float m_mouseY = 0;
    float m_mouseOffsetX = 0;
    float m_mouseOffsetY = 0;
    
    bool m_prevLeftMouseButtonState;
    bool m_currLeftMouseButtonState;
    
    bool m_prevRightMouseButtonState;
    bool m_currRightMouseButtonState;
};
