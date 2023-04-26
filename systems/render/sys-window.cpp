#include "pch.h"
#include "sys-window.h"
#include "sys-log.h"


namespace axiom {

Sys_Window::~Sys_Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

Sys_Window::Sys_Window(flecs::world &world_, std::string title, int w, int h)
{
  world = &world_;
  GLFWwindow* window;
  GLFWmonitor* primary;
  const GLFWvidmode* mode;

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  primary = glfwGetPrimaryMonitor();


  mode = glfwGetVideoMode(primary);
  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
  window = glfwCreateWindow(w,h,title.c_str(), nullptr, nullptr);

  world->set<Cmp_Window>({w,h,window, primary, mode});
}

void Sys_Window::Init()
{
  height_ = 720;
  width_ = 1280;
  maximized_ = false;

  if(!glfwInit()){
    //axiom::LogError("FAILED TO INITIALIZE GLFW");
    exit(EXIT_FAILURE);
  }

  //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  primary_ = glfwGetPrimaryMonitor();
  mode_ = glfwGetVideoMode(primary_);
  glfwWindowHint(GLFW_RED_BITS, mode_->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode_->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode_->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode_->refreshRate);

  window_ = glfwCreateWindow(width_, height_, "Axiom Engine", nullptr, nullptr);
  if(!window_){
    //axiom::LogError("Failed to Create Window");
  }
  glfwSetErrorCallback([](int error, const char* description){
    //axiom::LogError(description);
  });

  /*
  glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods){
    SWindow().SetKey(key);
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
  });*/

  glfwMakeContextCurrent(window_);
  //gladLoadGL(glfwGetProcAddress);
  
  
  glfwSwapInterval(1);
}

void Sys_Window::Update() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void Sys_Window::Maximize() {
  if (maximized_) {
    maximized_ = false;
    glfwSetWindowMonitor(window_, primary_, 0, 0, mode_->width, mode_->height, mode_->refreshRate);
  } else {
    maximized_ = true;
    glfwMaximizeWindow(window_);
  }
  Resize();
}

void Sys_Window::Resize() {
  glfwGetWindowSize(window_, &width_, &height_);
}

}  // namespace axiom
