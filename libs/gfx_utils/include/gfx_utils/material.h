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
  glm::vec3 ambient_color;
  glm::vec3 diffuse_color;
  glm::vec3 specular_color;
  glm::vec3 emission_color;

  float shininess;

  IllumModel illum;

  std::string ambient_texname;
  std::string diffuse_texname;
  std::string specular_texname;
};

}

#endif