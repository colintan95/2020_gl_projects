#include "gfx_utils/window/window.h"

#include <iostream>
#include <cassert>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// TODO(colintan): Define this somewhere else
const double kPi = 3.14159265358979323846;

namespace gfx_utils {

const int kGLVersionMajor = 3;
const int kGLVersionMinor = 3;
const int kGLFWSamples = 4;
const int kGLFWOpenGLProfile = GLFW_OPENGL_CORE_PROFILE;

const int kWindowWidth = 1024;
const int kWindowHeight = 768;
const char *kWindowTitle = "Hello Window";

Window::Window() : is_initialized_(false) {}

Window::~Window() {
  if (is_initialized_) {
    glfwTerminate();
  }
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

  // TODO: remove this
  view_mat_ = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -60.f)) *
              glm::rotate(glm::mat4(1.f), (static_cast<float>(kPi)/ 8.f),
                          glm::vec3(1.f, 0.f, 0.f));

  return true;
}

void Window::SwapBuffers() {
  assert(glfw_window_ != nullptr);

  glfwSwapBuffers(glfw_window_);
}

void Window::TickMainLoop() {
  glfwPollEvents();

  HandleKeyboardInput();
}

bool Window::ShouldQuit() {
  assert(glfw_window_ != nullptr);

  return (glfwWindowShouldClose(glfw_window_) == 1);
}

glm::mat4 Window::GetViewMatrix() {
  return view_mat_;
}

void Window::HandleKeyboardInput() {
  // TODO: remove after testing
  if (glfwGetKey(glfw_window_, GLFW_KEY_A) == GLFW_PRESS) {
    view_mat_ = glm::rotate(view_mat_, (static_cast<float>(kPi)/ 60.f),
                            glm::vec3(0.f, 1.f, 0.f));
  }
  if (glfwGetKey(glfw_window_, GLFW_KEY_D) == GLFW_PRESS) {
    view_mat_ = glm::rotate(view_mat_, -(static_cast<float>(kPi) / 60.f),
                            glm::vec3(0.f, 1.f, 0.f));
  }
}

}