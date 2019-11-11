#include "gfx_utils/window/window.h"

#include <iostream>
#include <cassert>

namespace gfx_utils {

const int kGLVersionMajor = 3;
const int kGLVersionMinor = 3;
const int kGLFWSamples = 4;
const int kGLFWOpenGLProfile = GLFW_OPENGL_CORE_PROFILE;

const int kWindowWidth = 1024;
const int kWindowHeight = 768;
const char *kWindowTitle = "Hello Window";

bool Window::has_instance_ = false;
Window *Window::instance_ptr_ = nullptr;

Window::Window() : is_initialized_(false) {
  assert(!has_instance_);
  assert(instance_ptr_ == nullptr);
  has_instance_ = true;
  instance_ptr_ = this;
}

Window::~Window() {
  if (is_initialized_) {
    glfwTerminate();
  }

  instance_ptr_ = nullptr;
  has_instance_ = false;
}

bool Window::Inititalize() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return false;
  }
  
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, kGLVersionMajor);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, kGLVersionMinor);
  glfwWindowHint(GLFW_OPENGL_PROFILE, kGLFWOpenGLProfile);
  glfwWindowHint(GLFW_SAMPLES, kGLFWSamples);

  // TODO(colintan): Check if we really need this
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  glfw_window_ = glfwCreateWindow(kWindowWidth, kWindowHeight, kWindowTitle,
                                  nullptr, nullptr);
  
  if (glfw_window_ == nullptr) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    return false;
  }

  glfwMakeContextCurrent(glfw_window_);

  // TODO(colintan): Is glewExperimental needed?
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Failed to initialize GLEW" << std::endl;
    return false;
  }

  is_initialized_ = true;

  glfwSetKeyCallback(glfw_window_, KeyboardInputCallback);
  glfwSetCursorPosCallback(glfw_window_, MouseInputCallback);

  return true;
}

void Window::SwapBuffers() {
  assert(glfw_window_ != nullptr);

  glfwSwapBuffers(glfw_window_);
}

void Window::TickMainLoop() {
  glfwPollEvents();

  TriggerKeyActions();
}

bool Window::ShouldQuit() {
  assert(glfw_window_ != nullptr);

  return (glfwWindowShouldClose(glfw_window_) == 1);
}

void Window::RegisterKeyBinding(int key, KeyActionType action,
                                std::function<void()> func) {
  KeyActionInfo action_info;
  action_info.type = action;
  action_info.func = func;
  key_action_map_[key] = action_info;
}

void Window::TriggerKeyActions() {
  for (auto &it : key_action_map_) {
    if (it.second.status) {
      it.second.func();

      if (it.second.type == KEY_ACTION_PRESS) {
        it.second.status = false;
      }
    }
  }
}

void Window::HandleKeyEvent(int key, KeyEventType event) {
  if (event == KEY_EVENT_DOWN || event == KEY_EVENT_UP) {
    bool status = (event == KEY_EVENT_DOWN) ? true : false;

    auto press_it = key_action_map_.find(key);
    if (press_it != key_action_map_.end()) {
      press_it->second.status = status;
    }
  }
}

void Window::KeyboardInputCallback(GLFWwindow* window, int key, int scancode, 
                                  int action, int mods) {
  if (action == GLFW_PRESS || action == GLFW_RELEASE) {
    KeyEventType event = (action == GLFW_PRESS) ? KEY_EVENT_DOWN : KEY_EVENT_UP;
    instance_ptr_->HandleKeyEvent(key, event);
  }
}

void Window::MouseInputCallback(GLFWwindow *window, double xpos, double ypos) {
}

} // namespace gfx_utils