#include "gfx_utils/window/window.h"

#include <iostream>
#include <cassert>
#include <array>
#include <utility>

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

  // TODO: remove this
  view_mat_ = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, -60.f)) *
              glm::rotate(glm::mat4(1.f), (static_cast<float>(kPi)/ 8.f),
                          glm::vec3(1.f, 0.f, 0.f));

  glfwSetKeyCallback(glfw_window_, KeyboardInputCallback);
  glfwSetCursorPosCallback(glfw_window_, MouseInputCallback);

  SetCameraPanMode();

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

glm::mat4 Window::GetViewMatrix() {
  return view_mat_;
}

void Window::PanCamera(CameraAction action) {
  assert(action == CAMERA_PAN_LEFT || action == CAMERA_PAN_RIGHT ||
         action == CAMERA_PAN_UP   || action == CAMERA_PAN_DOWN);

  float d = 1.f;

  glm::vec3 vect;
  switch (action) {
  case CAMERA_PAN_LEFT:
    vect = glm::vec3(d, 0.f, 0.f);
    break;
  case CAMERA_PAN_RIGHT:
    vect = glm::vec3(-d, 0.f, 0.f);
    break;
  case CAMERA_PAN_UP:
    vect = glm::vec3(0.f, -d, 0.f);
    break;
  case CAMERA_PAN_DOWN:
    vect = glm::vec3(0.f, d, 0.f);
    break;
  }

  view_mat_ = glm::translate(view_mat_, vect);
}

void Window::RotateCamera(CameraAction action) {
  assert(action == CAMERA_ROTATE_LEFT || action == CAMERA_ROTATE_RIGHT);
  float direction = 0.f;

  if (action == CAMERA_ROTATE_LEFT) {
    direction = 1.f;
  }
  else if (action == CAMERA_ROTATE_RIGHT) {
    direction = -1.f;
  }

  // TODO: this is wrong - the camera is rotating around the origin rather
  // than about itself
  view_mat_ = glm::rotate(view_mat_, direction * static_cast<float>(kPi) / 60.f,
                          glm::vec3(0.f, 1.f, 0.f));
}

void Window::ToggleCameraMode() {
  // if (camera_mode_ == CAMERA_PAN_MODE) {
  //   camera_mode_ = CAMERA_ROTATE_MODE;
  // }
  // else if (camera_mode_ == CAMERA_ROTATE_MODE) {
  //   camera_mode_ = CAMERA_PAN_MODE;
  // }
}

void Window::SetCameraPanMode() {
  camera_mode_ = CAMERA_PAN_MODE;

  std::array<std::pair<int, CameraAction>, 4> key_bindings =
      {{ {GLFW_KEY_A, CAMERA_PAN_LEFT}, {GLFW_KEY_D, CAMERA_PAN_RIGHT},
         {GLFW_KEY_W, CAMERA_PAN_UP},   {GLFW_KEY_S, CAMERA_PAN_DOWN} }};

  // Registers the keyboard keys to the associated action
  for (auto it : key_bindings) {
    KeyActionInfo action_info;
    action_info.type = KEY_ACTION_HOLD;
    action_info.func = [=]() {
      this->PanCamera(it.second);
    };
    key_action_map_[it.first] = action_info;
  }
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

}