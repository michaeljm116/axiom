#include "pch.h"
#include "sys-window.h"
#include "sys-log.h"

namespace axiom {
  namespace window{

    void Destruct() {
      glfwDestroyWindow(g_world.get_mut<Cmp_Window>()->window);
      glfwTerminate();
    }

    void Init(std::string title, int w, int h)
    {
      //Initialize GLFW
      glfwInit();
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      if(!glfwInit()){
        log::Set(LogLevel::ERROR, "FAILED TO INITIALIZE GLFW");
        exit(EXIT_FAILURE);
      }

      //Create Window
      //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

      GLFWmonitor* primary = glfwGetPrimaryMonitor();
      const GLFWvidmode* mode = glfwGetVideoMode(primary);

      glfwWindowHint(GLFW_RED_BITS, mode->redBits);
      glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
      glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
      glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

      //Set the singleton in world. 
      g_world.set<Cmp_Window>({w, h, title, glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr), primary, mode});

    auto* window = g_world.get<Cmp_Window>()->window;
    if(!window){
        axiom::log::Set(LogLevel::ERROR, "Failed to Create Window");
        exit(EXIT_FAILURE);
      }
      // Set call backs
      glfwSetErrorCallback([](int error, const char* description){
        axiom::log::Set(LogLevel::ERROR, description);
      });

      glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
      });
      glfwMakeContextCurrent(window);  
      glfwSwapInterval(1);

      //Start the Window System
      g_world.system<Cmp_Window>()
      .kind(flecs::OnUpdate)
      .each([](flecs::entity e, Cmp_Window& cmp_wind){
        glfwPollEvents();
      });

    }

    void Update(flecs::entity e, Cmp_Window& cmp_wind) {
      while (!glfwWindowShouldClose(cmp_wind.window)) {
        glfwPollEvents();
      }
    }


    void Resize(flecs::entity e, Cmp_Window& cmp_wind, Cmp_Window_Change& cmp_change) {
      if(cmp_change.setting == WindowSetting::FullScreen){
        glfwMaximizeWindow(cmp_wind.window);
      }
      if(cmp_change.setting == WindowSetting::Windowed){
        glfwSetWindowMonitor(cmp_wind.window, cmp_wind.primary, 0, 0, cmp_wind.mode->width, cmp_wind.mode->height, cmp_wind.mode->refreshRate);
      }
      glfwGetWindowSize(cmp_wind.window, &cmp_wind.width, &cmp_wind.height);
    }
  }
}  // namespace axiom
