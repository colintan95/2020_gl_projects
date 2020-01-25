#ifndef GFX_UTILS_WINDOW_CAMERA_H_
#define GFX_UTILS_WINDOW_CAMERA_H_

#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>

namespace gfx_utils {

// Forward declaration
class Window;

class Camera {
public:
  Camera();
  ~Camera();

  bool Initialize(Window *window);

  glm::mat4 CalcViewMatrix();
  glm::vec3 GetCameraLocation();

private:
  enum CameraAction {
    CAMERA_ROTATE_LEFT,
    CAMERA_ROTATE_RIGHT,
    CAMERA_ROTATE_UP,
    CAMERA_ROTATE_DOWN,
    CAMERA_PAN_LEFT,
    CAMERA_PAN_RIGHT,
    CAMERA_PAN_UP,
    CAMERA_PAN_DOWN,
    CAMERA_FPS_FORWARD,
    CAMERA_FPS_BACKWARD,
    CAMERA_FPS_LEFT,
    CAMERA_FPS_RIGHT
  };

  enum CameraMode {
    CAMERA_PAN_MODE,
    CAMERA_ROTATE_MODE,
    CAMERA_FPS_MODE
  };

private:
  void MouseMoveCallback(double x, double y);

  void PanCamera(CameraAction action);
  void RotateCamera(CameraAction action);
  void FpsMoveCamera(CameraAction action);

  void SetCameraMode(CameraMode mode);

  void ResetCameraMode();

  // TODO(colintan): Consider changing these to RotateLocalAxes() and
  // RotateGlobalAxes() where these functions take as params the axis (x,y,z) 
  // to rotate about and the angle change
  // May be needed if, for example, we may want to rotate about the y-axis
  // (yaw) at times by the global axes and at times by the local axes, not
  // just always one of them
  void RotateYaw(float angle_change);
  void RotatePitch(float angle_change);

  // Converts a vector from the local (camera) transform to a vector in the 
  // global transform
  glm::vec3 LocalToGlobalTransform(glm::vec3 vec_local);

private: 
  Window *window_;

  glm::quat camera_rotation_;
  glm::vec3 camera_loc_;
  
  CameraMode camera_mode_;

  double prev_mouse_x_;
  double prev_mouse_y_;

private:
  friend class Window;
};

} // namespace gfx_utils

#endif