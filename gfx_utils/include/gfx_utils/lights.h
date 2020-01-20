#ifndef GFX_UTILS_LIGHTS_H_
#define GFX_UTILS_LIGHTS_H_

#include <string>

#include <glm/matrix.hpp>

namespace gfx_utils {

struct Light {
  // TODO(colintan): Put this in a macro - so that we just need to call
  // the macro for future light structs that we create
  static const char* GetType() {
    return "none";
  }
};

struct Spotlight : Light {
  glm::vec3 position;
  glm::vec3 diffuse_intensity;
  glm::vec3 specular_intensity;

  glm::vec3 direction;
  float cone_angle;

  // To orient the camera for shadow mapping
  glm::vec3 camera_up;

  static const char* GetType() {
    return "spotlight";
  }
};


} // namespace gfx_utils

#endif // GFX_UTILS_LIGHTS_H_