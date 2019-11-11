#include "gfx_utils/window/camera.h"

#include <cassert>
#include <array>

#include "gfx_utils/window/window.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// TODO(colintan): Define this somewhere else
const double kPi = 3.14159265358979323846;

namespace gfx_utils {

Camera::Camera() : window_(nullptr),
                   camera_loc_(0.f, 0.f, 0.f), 
                   camera_yaw_(0.f),
                   camera_pitch_(0.f),
                   camera_roll_(0.f),
                   camera_mode_(CAMERA_PAN_MODE) {
}

Camera::~Camera() {
  window_ = nullptr;
}

bool Camera::Initialize(Window *window) {
  assert(window != nullptr);

  window_ = window;

  // TODO: remove this
  camera_loc_ = glm::vec3(0.f, 20.f, 60.f);
  camera_pitch_ = -(static_cast<float>(kPi) / 8.f);

  SetCameraPanMode();

  window_->RegisterKeyBinding(GLFW_KEY_Z, KEY_ACTION_PRESS, [this]() {
    this->ToggleCameraMode();
  });

  return true;
}

glm::mat4 Camera::CalcViewMatrix() {
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

void Camera::PanCamera(CameraAction action) {
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

void Camera::RotateCamera(CameraAction action) {
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

void Camera::ToggleCameraMode() {
  if (camera_mode_ == CAMERA_PAN_MODE) {
    SetCameraRotateMode();
  }
  else if (camera_mode_ == CAMERA_ROTATE_MODE) {
    SetCameraPanMode();
  }
}

// TODO: define the key bindings for both modes in a larger map so that
// we don't have to duplicate this code
void Camera::SetCameraPanMode() {
  camera_mode_ = CAMERA_PAN_MODE;

  std::array<std::pair<int, CameraAction>, 4> key_bindings =
      {{ {GLFW_KEY_A, CAMERA_PAN_LEFT}, {GLFW_KEY_D, CAMERA_PAN_RIGHT},
         {GLFW_KEY_W, CAMERA_PAN_UP},   {GLFW_KEY_S, CAMERA_PAN_DOWN} }};

  // Registers the keyboard keys to the associated action
  for (auto it : key_bindings) {
    window_->RegisterKeyBinding(it.first, KEY_ACTION_HOLD, [=]() {
      this->PanCamera(it.second);
    });
  }
}

void Camera::SetCameraRotateMode() {
  camera_mode_ = CAMERA_ROTATE_MODE;

  std::array<std::pair<int, CameraAction>, 4> key_bindings =
      {{ {GLFW_KEY_A, CAMERA_ROTATE_LEFT}, {GLFW_KEY_D, CAMERA_ROTATE_RIGHT},
         {GLFW_KEY_W, CAMERA_ROTATE_UP},   {GLFW_KEY_S, CAMERA_ROTATE_DOWN} }};

  // Registers the keyboard keys to the associated action
  for (auto it : key_bindings) {
    window_->RegisterKeyBinding(it.first, KEY_ACTION_HOLD, [=]() {
      this->RotateCamera(it.second);
    });
  }
}

} // namespace gfx_utils