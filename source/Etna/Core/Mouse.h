#pragma once
#include <glm/glm.hpp>
#include "Vulkan/VulkanHeader.h"

class Mouse 
{
public:
    enum class Mode
    {
        Relative,
        Absolute
    };

    struct State
    {
        Mouse::Mode mode;
        glm::vec2 position;
        float wheelDelta;
        bool leftButtonPressed;
        bool leftButtonReleased;
        bool rightButtonPressed;
        bool rightButtonReleased;
    };

public:
    Mouse(GLFWwindow* window);

    void ToggleMode();

    State GetState();

private:
    bool IsButtonPressed(int button) const;
    bool IsButtonReleased(int button) const;

    static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

private:
    GLFWwindow* m_Window;
    glm::vec2   m_LastPosition;
    Mode        m_Mode;
    
    static float m_WheelDelta;
};
