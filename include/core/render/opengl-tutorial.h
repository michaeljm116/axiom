#pragma once

#include "shaders.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace axiom {

class OpenGLTutorial {
public:
    OpenGLTutorial(GLFWwindow* window);
    ~OpenGLTutorial();

    void Init();
    void Render();
    void Update();
    
    void ScreenShot();
    void LoadTexture();

    void InitImGUI();

private:
    GLFWwindow* window_;
    GLuint vao_;
    GLuint program_;

    GLuint handle_vbo;
    GLuint handle_elements;


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

    struct TextureTutorial{
        int w, h, comp;
        GLuint texture;
        uint8_t* img;
    }tut;

    struct ImDrawVert{
        ImVec2 pos;
        ImVec2 uv;
        ImU32 col;
    };
};

} // namespace axiom
