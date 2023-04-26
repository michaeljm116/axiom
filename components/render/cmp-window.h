#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace axiom{
    struct Cmp_Window{
        int width = 1280;
        int height = 720;
        std::string name;
        GLFWwindow* window;
        GLFWmonitor* primary;
        const GLFWvidmode* mode;
    };

    enum class WindowSetting{
        Resize,
        Windowed,
        FullScreen,
        WindowedFullScreen
    };

    struct Cmp_Window_Change{
        WindowSetting setting;
    };
}