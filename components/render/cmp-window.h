/**
 * @file cmp-window.h
 * @author Mike Murrell (mikestoleyobike@aim.com)
 * @brief Window Components
 * @version 0.1
 * @date 2023-05-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <GLFW/glfw3.h>
#include <string>

namespace Axiom{
    namespace Window{
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
        Window::Setting setting;
    };



}