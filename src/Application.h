//
//  Application.hpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.11.24.
//

#pragma once

struct Application
{
    Application();
    ~Application();
    
    void run();
    
    static void setCursorEnabled(bool state);
    static void quit();
    
private:
    struct GLFWwindow* m_pWindow;
    
    class Camera* m_pCamera;
    class Q3MapScene* m_pScene;
    
    void updateInputState();
    
    static Application* currentInstance;
};
