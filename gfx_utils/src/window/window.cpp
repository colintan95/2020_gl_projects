#include "gfx_utils/window/window.h"

#include <iostream>
#include <cassert>
#include <array>
#include <utility>
#include <cmath>

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

Window::Window() : is_initialized_(false), camera_loc_(0.f, 0.f, 0.f),
                   camera_yaw_(0.f), camera_pitch_(0.f), camera_roll_(0.f) {
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
  camera_loc_ = glm::vec3(0.f, 20.f, 60.f);
  camera_pitch_ = -(static_cast<float>(kPi) / 8.f);

  glfwSetKeyCallback(glfw_window_, KeyboardInputCallback);
  glfwSetCursorPosCallback(glfw_window_, MouseInputCallback);

  SetCameraPanMode();

  // TODO: do this somewhere else
  KeyActionInfo action_info;
  action_info.type = KEY_ACTION_PRESS;
  action_info.func = [this]() {
    this->ToggleCameraMode();
  };
  key_action_map_[GLFW_KEY_Z] = action_info;

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

glm::mat4 Window::CalcViewMatrix() {
  glm::mat4 view_mat = glm::mat4(1.f);
  
  view_mat *= glm::rotate(glm::mat4(1.f), -camera_roll_, 
                          glm::vec3(0.f, 0.f, 1.f));
  view_mat *= glm::rotate(glm::mat4(1.f), -camera_pitch_, 
                          glm::vec3(1.f, 0.f, 0.f));
  view_mat *= glm::rotate(glm::mat4(1.f), -camera_yaw_,
                          glm::vec3(0.f, 1.f, 0.f));
  view_mat *= glm::translate(glm::mat4(1.f), -camera_loc_);

  return view_mat;
}

void Window::PanCamera(CameraAction action) {
  assert(action == CAMERA_PAN_LEFT || action == CAMERA_PAN_RIGHT ||
         action == CAMERA_PAN_UP   || action == CAMERA_PAN_DOWN);

  float d = 1.f;

  switch (action) {
  case CAMERA_PAN_LEFT:
    camera_loc_ += 
        glm::vec3(glm::rotate(glm::mat4(1.f), camera_yaw_,
                              glm::vec3(0.f, 1.f, 0.f)) * 
                      glm::vec4(-d, 0.f, 0.f, 0.f));
    break;
  case CAMERA_PAN_RIGHT:
    camera_loc_ +=
        glm::vec3(glm::rotate(glm::mat4(1.f), camera_yaw_,
                              glm::vec3(0.f, 1.f, 0.f)) * 
                      glm::vec4(d, 0.f, 0.f, 0.f));
    break;
  case CAMERA_PAN_UP:
    camera_loc_ += glm::vec3(0.f, d, 0.f);
    break;
  case CAMERA_PAN_DOWN:
    camera_loc_ += glm::vec3(0.f, -d, 0.f);
    break;
  }
}

void Window::RotateCamera(CameraAction action) {
  assert(action == CAMERA_ROTATE_LEFT || action == CAMERA_ROTATE_RIGHT ||
         action == CAMERA_ROTATE_UP   || action == CAMERA_ROTATE_DOWN);

  float d = static_cast<float>(kPi) / 120.f;

  // TODO: create an angle type to handle angle wrapping
  switch (action) {
  case CAMERA_ROTATE_LEFT:
    camera_yaw_ += d;
    break;
  case CAMERA_ROTATE_RIGHT:
    camera_yaw_ -= d;  
    break;
  case CAMERA_ROTATE_UP:
    camera_pitch_ += d;
    break;
  case CAMERA_ROTATE_DOWN:
    camera_pitch_ -= d;
    break;
  }
}

void Window::ToggleCameraMode() {
  if (camera_mode_ == CAMERA_PAN_MODE) {
    SetCameraRotateMode();
  }
  else if (camera_mode_ == CAMERA_ROTATE_MODE) {
    SetCameraPanMode();
  }
}

// TODO: define the key bindings for both modes in a larger map so that
// we don't have to duplicate this code
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

void Window::SetCameraRotateMode() {
  camera_mode_ = CAMERA_ROTATE_MODE;

  std::array<std::pair<int, CameraAction>, 4> key_bindings =
      {{ {GLFW_KEY_A, CAMERA_ROTATE_LEFT}, {GLFW_KEY_D, CAMERA_ROTATE_RIGHT},
         {GLFW_KEY_W, CAMERA_ROTATE_UP},   {GLFW_KEY_S, CAMERA_ROTATE_DOWN} }};

  // Registers the keyboard keys to the associated action
  for (auto it : key_bindings) {
    KeyActionInfo action_info;
    action_info.type = KEY_ACTION_HOLD;
    action_info.func = [=]() {
      this->RotateCamera(it.second);
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

} // namespace gfx_utils