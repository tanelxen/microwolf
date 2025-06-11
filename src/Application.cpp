//
//  Application.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.11.24.
//

#include <stdio.h>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Q3MapScene.h"

#include "Camera.h"

#include "Input.h"
#include "HUD.h"

#include "Application.h"

static bool isInMenu = true;

static void error_callback(int e, const char *d) { printf("Error %d: %s\n", e, d); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        isInMenu = !isInMenu;
        Application::setCursorEnabled(isInMenu);
    }
}

Application* Application::currentInstance = nullptr;

void imgui_init(GLFWwindow* window);
void imgui_draw();

Application::Application()
{
    /* GLFW */
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE);
#endif

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
//    glfwWindowHint(GLFW_SAMPLES, 8);

    const int window_width = 1440;
    const int window_height = 900;
    m_pWindow = glfwCreateWindow(window_width, window_height, "Micro-Wolf", nullptr, nullptr);

    glfwSetKeyCallback(m_pWindow, key_callback);

    glfwMakeContextCurrent(m_pWindow);

    gladLoadGL();

    const unsigned char* version = glGetString(GL_VERSION);
    printf("version: %s\n", version);
    
    const unsigned char* device = glGetString(GL_RENDERER);
    printf("device: %s\n", device);
    
    imgui_init(m_pWindow);
    
    m_pCamera = new Camera();
    m_pScene = new Q3MapScene(m_pCamera);
    
    glfwSwapInterval(0);
    glFrontFace(GL_CCW);
    
    currentInstance = this;
    
    Application::setCursorEnabled(isInMenu);
}

Application::~Application()
{
    delete m_pScene;
    delete m_pCamera;
    
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}

void Application::run()
{
    HUD hud;
    hud.init();
    
    double lastTime = 0;
    const double desiredFrameTime = 1.0 / 60;
    
//    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glEnable(GL_FRAMEBUFFER_SRGB);
    
    while (!glfwWindowShouldClose(m_pWindow))
    {
        double currTime = glfwGetTime();
        double elapsedTime = currTime - lastTime;
        
        if (elapsedTime < desiredFrameTime)
        {
            long long diff = (desiredFrameTime - elapsedTime) * 1000;
            std::this_thread::sleep_for(std::chrono::milliseconds(diff));
            continue;
        }
        
        lastTime = currTime;

        int width, height;
        glfwGetFramebufferSize(m_pWindow, &width, &height);

        m_pCamera->setAspectRatio((float)width / (float)height);
        m_pScene->update(elapsedTime);
        
        glViewport(0, 0, width, height);
        
        m_pScene->draw();
        
        hud.resize(width, height);
        hud.draw();
        
        imgui_draw();

        glfwSwapBuffers(m_pWindow);
        
        glfwPollEvents();
        updateInputState();
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}


void Application::updateInputState()
{
    static bool isFirstFrame = true;
    
    if (isInMenu)
    {
        isFirstFrame = true;
        return;
    }
    
    for (int i = 32; i < 256; ++i)
    {
        Input::getInstance().prev_keys[i] = Input::getInstance().keys[i];
        Input::getInstance().keys[i] = glfwGetKey(m_pWindow, i) > GLFW_RELEASE;
    }
    
    Input::getInstance().m_prevLeftMouseButtonState = Input::getInstance().m_currLeftMouseButtonState;
    Input::getInstance().m_prevRightMouseButtonState = Input::getInstance().m_currRightMouseButtonState;
    
    Input::getInstance().m_currLeftMouseButtonState = glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) > GLFW_RELEASE;
    Input::getInstance().m_currRightMouseButtonState = glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_RIGHT) > GLFW_RELEASE;
    
    
    double mouseX, mouseY;
    glfwGetCursorPos(m_pWindow, &mouseX, &mouseY);
    
    if (isFirstFrame)
    {
        Input::getInstance().m_mouseOffsetX = 0;
        Input::getInstance().m_mouseOffsetY = 0;
    }
    else
    {
        Input::getInstance().m_mouseOffsetX = mouseX - Input::getInstance().m_mouseX;
        Input::getInstance().m_mouseOffsetY = mouseY - Input::getInstance().m_mouseY;
    }
    
    Input::getInstance().m_mouseX = mouseX;
    Input::getInstance().m_mouseY = mouseY;
    
    isFirstFrame = false;
}

void Application::setCursorEnabled(bool state)
{
    if(currentInstance)
    {
        glfwSetInputMode(currentInstance->m_pWindow, GLFW_CURSOR, state ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}

void Application::quit()
{
    if(currentInstance)
    {
        glfwSetWindowShouldClose(currentInstance->m_pWindow, GLFW_TRUE);
    }
}

void imgui_init(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
    
    //    setImGuiStyle();
}

void imgui_draw()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    if (isInMenu)
    {
        static float f = 0.0f;
        ImGui::Text("Press ESC to switch between DEBUG and GAME mode");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        
        if (ImGui::Button("Quit"))
        {
            Application::quit();
        }
    }
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
