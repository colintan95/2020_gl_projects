#ifndef GFX_UTILS_MATERIAL_H_
#define GFX_UTILS_MATERIAL_H_

#include <glm/vec3.hpp>

#include <string>

#include "texture.h"

namespace gfx_utils {

enum IllumModel {
  kIllumModelInvalid,
  kIllumModelColorOnly,
  kIllumModelAmbientOnly,
  kIllumModelHighlight
};

struct Material {
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  glm::vec3 emission;

  float shininess;

  IllumModel illum;

  std::string texture_path;
};

}

#endif