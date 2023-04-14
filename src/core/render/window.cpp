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

  glfwInit();
  //glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

  primary_ = glfwGetPrimaryMonitor();
  mode_ = glfwGetVideoMode(primary_);
  glfwWindowHint(GLFW_RED_BITS, mode_->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode_->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode_->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode_->refreshRate);

  window_ = glfwCreateWindow(width_, height_, "Axiom Engine", nullptr, nullptr);

  glfwSetErrorCallback([](int error, const char* description){
    axiom::LogError(description);
  });
  glfwSetKeyCallback(window_, [](GLFWwindow* window, int key, int scancode, int action, int mods){
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
  });
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
