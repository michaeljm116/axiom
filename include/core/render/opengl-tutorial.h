#pragma once

#include "window.h"
#include <GLFW/glfw3.h>

namespace axiom {

class OpenGLTutorial {
public:
    OpenGLTutorial(GLFWwindow* window);
    ~OpenGLTutorial();

    void Init();
    void Render();

private:
    GLFWwindow* window_;
    GLuint vao_;
    GLuint program_;
};

} // namespace axiom
