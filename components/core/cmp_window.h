#pragma once
#include <GLFW/glfw3.h>

namespace axiom{
    struct Cmp_Window{
        int width = 1280;
        int height = 720;
        GLFWwindow* window;
        GLFWmonitor* primary;
        const GLFWvidmode* mode;
    };

    enum class WindowSetting{
        None,
        Maximize,
        Resize,
        WindowedFullScreen
    };

    struct Cmp_Window_Change{
        WindowSetting setting;
    };
}