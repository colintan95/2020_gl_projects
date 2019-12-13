#ifndef GFX_UTILS_LIGHTS_H_
#define GFX_UTILS_LIGHTS_H_

#include <glm/matrix.hpp>

namespace gfx_utils {

struct SpotLight {
  glm::vec3 position;
  glm::vec3 diffuse_intensity;
  glm::vec3 specular_intensity;

  glm::vec3 direction;
  float cone_angle;
};

} // namespace gfx_utils

#endif // GFX_UTILS_LIGHTS_H_