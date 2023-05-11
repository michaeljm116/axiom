#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace axiom{
    namespace window{
    enum class Setting{
        Resize,
        Windowed,
        FullScreen,
        WindowedFullScreen
    };};

    struct Cmp_Window{
        int width = 1280;
        int height = 720;
        std::string name;
        GLFWwindow* window;
        GLFWmonitor* primary;
        const GLFWvidmode* mode;
    };

    struct Cmp_Window_Change{
        window::Setting setting;
    };



}