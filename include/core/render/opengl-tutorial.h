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
    void Update();

private:
    GLFWwindow* window_;
    GLuint vao_;
    GLuint program_;


    int width_;
    int height_;
    float ratio_;
    glm::mat4 m_;
    glm::mat4 p_;

    struct PerFrameData{
        glm::mat4 mvp_;
        int is_wireframe;
    }per_frame_data;
    const GLsizeiptr k_buffer_size = sizeof(PerFrameData);
    GLuint per_frame_data_buf;
};

} // namespace axiom