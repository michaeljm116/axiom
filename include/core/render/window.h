#pragma once

#include <GLFW/glfw3.h>
#include <cstdint>

namespace axiom {

class Window {
 public:
  static Window& GetInstance() {
    static Window instance;
    return instance;
  }

  void Init();
  void Update();
  void Maximize();
  void Resize();

  [[nodiscard]] GLFWwindow*  GetWindow() const { return window_; }
  [[nodiscard]] GLFWmonitor* GetMonitor() const { return primary_; }
  [[nodiscard]] int          GetWidth() const { return width_; }
  [[nodiscard]] int          GetHeight() const { return height_; }

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  ~Window();

 private:
  Window() = default;

  GLFWwindow* window_{nullptr};
  GLFWmonitor* primary_{nullptr};
  int width_{0};
  int height_{0};
  bool maximized_{false};
  const GLFWvidmode* mode_{nullptr};
};

// Add this function to provide a shorthand for calling the singleton
inline Window& SWindow() {
  return Window::GetInstance();
}

}  // namespace axiom
