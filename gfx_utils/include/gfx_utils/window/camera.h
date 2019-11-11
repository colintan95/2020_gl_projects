#ifndef GFX_UTILS_WINDOW_CAMERA_H_
#define GFX_UTILS_WINDOW_CAMERA_H_

#include <glm/matrix.hpp>

namespace gfx_utils {

// Forward declaration
class Window;

class Camera {
public:
  Camera();
  ~Camera();

  bool Initialize(Window *window);

  glm::mat4 CalcViewMatrix();

private:
  enum CameraAction {
    CAMERA_ROTATE_LEFT,
    CAMERA_ROTATE_RIGHT,
    CAMERA_ROTATE_UP,
    CAMERA_ROTATE_DOWN,
    CAMERA_PAN_LEFT,
    CAMERA_PAN_RIGHT,
    CAMERA_PAN_UP,
    CAMERA_PAN_DOWN
  };

  enum CameraMode {
    CAMERA_PAN_MODE,
    CAMERA_ROTATE_MODE
  };

private:
  void PanCamera(CameraAction action);
  void RotateCamera(CameraAction action);
  void ToggleCameraMode();

  void SetCameraPanMode();
  void SetCameraRotateMode();

private:
  Window *window_;

  glm::vec3 camera_loc_;
  float camera_yaw_;
  float camera_pitch_;
  float camera_roll_;
  CameraMode camera_mode_;

private:
  friend class Window;
};

} // namespace gfx_utils

#endif