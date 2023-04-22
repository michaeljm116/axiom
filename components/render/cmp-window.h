#pragma once
#include <GLFW/glfw3.h>
namespace axiom{
    struct Cmp_Window{
        GLFWwindow* window;
        GLFWmonitor* monitor;
        const GLFWvidmode* mode;
        
        int width = 0;
        int height = 0;
        bool maximized = false;
        int key_pressed = 0;
    }
}
