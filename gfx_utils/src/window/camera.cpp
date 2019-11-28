#include "gfx_utils/window/camera.h"

#include <cassert>
#include <array>
#include <limits>

#include "gfx_utils/window/window.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// TODO(colintan): remove
#include <iostream>

// TODO(colintan): Define this somewhere else
const double kPi = 3.14159265358979323846;

const double kMouseLocNoValue = std::numeric_limits<double>::max();
const float kFpsModeLookSpeed = 0.001f;

namespace gfx_utils {

Camera::Camera() : window_(nullptr),
                   camera_loc_(0.f, 0.f, 0.f), 
                   camera_rotation_(1.f, 0.f, 0.f, 0.f),
                   camera_mode_(CAMERA_PAN_MODE),
                   prev_mouse_x_(kMouseLocNoValue),
                   prev_mouse_y_(kMouseLocNoValue) {
}

Camera::~Camera() {
  window_ = nullptr;
}

bool Camera::Initialize(Window *window) {
  assert(window != nullptr);

  window_ = window;

  SetCameraMode(CAMERA_PAN_MODE);

  window_->RegisterKeyBinding(GLFW_KEY_Z, KEY_ACTION_PRESS, [this]() {
    this->SetCameraMode(CAMERA_PAN_MODE);
  });
  window_->RegisterKeyBinding(GLFW_KEY_X, KEY_ACTION_PRESS, [this]() {
    this->SetCameraMode(CAMERA_ROTATE_MODE);
  });
  window_->RegisterKeyBinding(GLFW_KEY_C, KEY_ACTION_PRESS, [this]() {
    this->SetCameraMode(CAMERA_FPS_MODE);
  });

  return true;
}

glm::mat4 Camera::CalcViewMatrix() {
  glm::mat4 view_mat = glm::mat4(1.f);

  // TODO(colintan): Is the inverse operation expensive?
  view_mat = glm::inverse(glm::mat4_cast(camera_rotation_)) *
             glm::translate(glm::mat4(1.f), -camera_loc_);
 
  return view_mat;
}

void Camera::PanCamera(CameraAction action) {
  assert(action == CAMERA_PAN_LEFT || action == CAMERA_PAN_RIGHT ||
         action == CAMERA_PAN_UP   || action == CAMERA_PAN_DOWN);

  float d = 0.5f;

  switch (action) {
  case CAMERA_PAN_LEFT:
    camera_loc_ += LocalToGlobalTransform(glm::vec3(-d, 0.f, 0.f));                 
    break;
  case CAMERA_PAN_RIGHT:
    camera_loc_ += LocalToGlobalTransform(glm::vec3(d, 0.f, 0.f));
    break;
  case CAMERA_PAN_UP:
    camera_loc_ += LocalToGlobalTransform(glm::vec3(0.f, d, 0.f));
    break;
  case CAMERA_PAN_DOWN:
    camera_loc_ += LocalToGlobalTransform(glm::vec3(0.f, -d, 0.f));
    break;
  }
}

void Camera::RotateCamera(CameraAction action) {
  assert(action == CAMERA_ROTATE_LEFT || action == CAMERA_ROTATE_RIGHT ||
         action == CAMERA_ROTATE_UP   || action == CAMERA_ROTATE_DOWN);

  float d = static_cast<float>(kPi) / 120.f;

  switch (action) {
  case CAMERA_ROTATE_LEFT:
    RotateYaw(d);
    break;
  case CAMERA_ROTATE_RIGHT:
    RotateYaw(-d);
    break;
  case CAMERA_ROTATE_UP:
    RotatePitch(d);
    break;
  case CAMERA_ROTATE_DOWN:
    RotatePitch(-d);
    break;
  }
}

void Camera::FpsMoveCamera(CameraAction action) {
  assert(action == CAMERA_FPS_LEFT    || action == CAMERA_FPS_RIGHT ||
         action == CAMERA_FPS_FORWARD || action == CAMERA_FPS_BACKWARD);

  float walk_speed = 0.8f;
  float strafe_speed = 0.5f;

  switch (action) {
  case CAMERA_FPS_LEFT:
    camera_loc_ += LocalToGlobalTransform(glm::vec3(-strafe_speed, 0.f, 0.f));
    break;
  case CAMERA_FPS_RIGHT:
    camera_loc_ += LocalToGlobalTransform(glm::vec3(strafe_speed, 0.f, 0.f));
    break;
  case CAMERA_FPS_FORWARD:
    camera_loc_ += LocalToGlobalTransform(glm::vec3(0.f, 0.f, -walk_speed));
    break;
  case CAMERA_FPS_BACKWARD:
    camera_loc_ += LocalToGlobalTransform(glm::vec3(0.f, 0.f, walk_speed));
    break;
  }
}

void Camera::SetCameraMode(CameraMode mode) {
  ResetCameraMode();

  if (mode == CAMERA_PAN_MODE) {
    camera_mode_ = CAMERA_PAN_MODE;

    std::array<std::pair<int, CameraAction>, 4> key_bindings =
        {{ {GLFW_KEY_A, CAMERA_PAN_LEFT}, {GLFW_KEY_D, CAMERA_PAN_RIGHT},
           {GLFW_KEY_W, CAMERA_PAN_UP},   {GLFW_KEY_S, CAMERA_PAN_DOWN} }};

    for (auto it : key_bindings) {
      window_->RegisterKeyBinding(it.first, KEY_ACTION_HOLD, [=]() {
        this->PanCamera(it.second);
      });
    }
  }
  else if (mode == CAMERA_ROTATE_MODE) {
    camera_mode_ = CAMERA_ROTATE_MODE;

    std::array<std::pair<int, CameraAction>, 4> key_bindings =
        {{ {GLFW_KEY_A, CAMERA_ROTATE_LEFT}, 
           {GLFW_KEY_D, CAMERA_ROTATE_RIGHT},
           {GLFW_KEY_W, CAMERA_ROTATE_UP},   
           {GLFW_KEY_S, CAMERA_ROTATE_DOWN} }};

    for (auto it : key_bindings) {
      window_->RegisterKeyBinding(it.first, KEY_ACTION_HOLD, [=]() {
        this->RotateCamera(it.second);
      });
    }
  }
  else if (mode == CAMERA_FPS_MODE) {
    camera_mode_ = CAMERA_FPS_MODE;

    std::array<std::pair<int, CameraAction>, 4> key_bindings =
        {{ {GLFW_KEY_A, CAMERA_FPS_LEFT}, 
           {GLFW_KEY_D, CAMERA_FPS_RIGHT},
           {GLFW_KEY_W, CAMERA_FPS_FORWARD}, 
           {GLFW_KEY_S, CAMERA_FPS_BACKWARD} }};

    for (auto it : key_bindings) {
      window_->RegisterKeyBinding(it.first, KEY_ACTION_HOLD, [=]() {
        this->FpsMoveCamera(it.second);
      });
    }  

    window_->RegisterMouseMoveBinding(static_cast<void*>(this), 
        [=](double x, double y) {
          this->MouseMoveCallback(x, y);
        });
  }
}

void Camera::MouseMoveCallback(double x, double y) {
  if (prev_mouse_x_ != kMouseLocNoValue && prev_mouse_y_ != kMouseLocNoValue) {
    float yaw_d = -static_cast<float>(x - prev_mouse_x_) * kFpsModeLookSpeed;
    float pitch_d = -static_cast<float>(y - prev_mouse_y_) * kFpsModeLookSpeed;

    RotateYaw(yaw_d);
    RotatePitch(pitch_d);
  }

  prev_mouse_x_ = x;
  prev_mouse_y_ = y;
}

void Camera::ResetCameraMode() {
  std::array<int, 4> keys = {GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_W, GLFW_KEY_S};

  for (int key : keys) {
    window_->RemoveKeyBinding(key);
  }

  window_->RemoveMouseMoveBinding(static_cast<void*>(this));

  prev_mouse_x_ = kMouseLocNoValue;
  prev_mouse_y_ = kMouseLocNoValue;
}

void Camera::RotateYaw(float angle_change) {
  // Yaw rotation is relative to the world coordinates
  camera_rotation_ = glm::rotate(glm::quat(1.f, 0.f, 0.f, 0.f), angle_change,
                                 glm::vec3(0.f, 1.f, 0.f)) *
                     camera_rotation_;
}

void Camera::RotatePitch(float angle_change) {
  // Pitch rotation relative to the local coordinates of the camera
  camera_rotation_ = glm::rotate(camera_rotation_, angle_change,
                                 glm::vec3(1.f, 0.f, 0.f));
}

glm::vec3 Camera::LocalToGlobalTransform(glm::vec3 vec_local) {
  return glm::mat3_cast(camera_rotation_) * vec_local;
}

} // namespace gfx_utils