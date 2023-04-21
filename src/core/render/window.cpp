#include "pch.h"
#include "core/render/window.h"
#include "core/util/log.h"

namespace axiom {

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

void Window::Init() {
  height_ = 720;
  width_ = 1280;
  maximized_ = false;

  if(!glfwInit()){
    axiom::LogError("FAILED TO INITIALIZE GLFW");
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
    axiom::LogError("Failed to Create Window");
  }
  glfwSetErrorCallback([](int error, const char* description){
    axiom::LogError(description);
  });
  glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods){
    SWindow().SetKey(key);
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
  });

  glfwMakeContextCurrent(window_);
  //gladLoadGL(glfwGetProcAddress);
  
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      axiom::LogError("Failed to initialize GLAD");
  }
  glfwSwapInterval(1);
}

void Window::Update() {
  while (!glfwWindowShouldClose(window_)) {
    glfwPollEvents();
  }
}

void Window::Maximize() {
  if (maximized_) {
    maximized_ = false;
    glfwSetWindowMonitor(window_, primary_, 0, 0, mode_->width, mode_->height, mode_->refreshRate);
  } else {
    maximized_ = true;
    glfwMaximizeWindow(window_);
  }
  Resize();
}

void Window::Resize() {
  glfwGetWindowSize(window_, &width_, &height_);
}

}  // namespace axiom
