//
//  Input.cpp
//  TryOpenGL
//
//  Created by Fedor Artemenkov on 30.11.24.
//

#include "Input.h"


bool Input::isKeyPressed(int keyNum)
{
    if (keyNum >= 256) return false;
    return getInstance().keys[keyNum];
}

bool Input::isKeyClicked(int keyNum)
{
    if (keyNum >= 256) return false;
    return !getInstance().keys[keyNum] && getInstance().prev_keys[keyNum];
}

bool Input::isLeftMouseButtonPressed()
{
    return getInstance().m_currLeftMouseButtonState;
}

bool Input::isRightMouseButtonPressed()
{
    return getInstance().m_currRightMouseButtonState;
}

bool Input::isLeftMouseButtonClicked()
{
    return !getInstance().m_currLeftMouseButtonState && getInstance().m_prevLeftMouseButtonState;
}

bool Input::isRightMouseButtonClicked()
{
    return !getInstance().m_currRightMouseButtonState && getInstance().m_prevRightMouseButtonState;
}

float Input::getMouseOffsetX()
{
    return getInstance().m_mouseOffsetX;
}

float Input::getMouseOffsetY()
{
    return getInstance().m_mouseOffsetY;
}

float Input::getMousePositionX()
{
    return getInstance().m_mouseX;
}

float Input::getMousePositionY()
{
    return getInstance().m_mouseY;
}
