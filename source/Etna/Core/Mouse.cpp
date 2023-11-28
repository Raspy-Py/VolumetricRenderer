#include "Mouse.h"

float Mouse::m_WheelDelta = 0.0f;

Mouse::Mouse(GLFWwindow* window)
    : m_Window(window), 
    m_LastPosition(0.0f, 0.0f), 
    m_Mode(Mode::Absolute)
{
    glfwSetScrollCallback(window, Mouse::ScrollCallback);
}

void Mouse::ToggleMode()
{
    if (m_Mode == Mode::Absolute)
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_Mode = Mode::Relative;
    }
    else
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_Mode = Mode::Absolute;
    }
}

Mouse::State Mouse::GetState()
{
    State state{};
    double currentX, currentY;
    glfwGetCursorPos(m_Window, &currentX, &currentY);

    state.mode = m_Mode;
    if (state.mode == Mode::Relative)
    {
        state.position.x = currentX - m_LastPosition.x;
        state.position.y = currentY - m_LastPosition.y;
    }
    else 
    {
        state.position.x = currentX;
        state.position.y = currentY;
    }

    m_LastPosition.x = currentX;
    m_LastPosition.y = currentY;

    state.leftButtonPressed = IsButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    state.leftButtonReleased = IsButtonReleased(GLFW_MOUSE_BUTTON_LEFT);
    state.rightButtonPressed = IsButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
    state.rightButtonReleased = IsButtonReleased(GLFW_MOUSE_BUTTON_RIGHT);

    state.wheelDelta = m_WheelDelta;
    m_WheelDelta = 0;

    return state;
}

bool Mouse::IsButtonPressed(int button) const 
{
    return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
}

bool Mouse::IsButtonReleased(int button) const 
{
    return glfwGetMouseButton(m_Window, button) == GLFW_RELEASE;
}

void Mouse::ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    m_WheelDelta += yOffset; 
}
