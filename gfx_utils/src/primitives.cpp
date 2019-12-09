#include "gfx_utils/primitives.h"

#include <glm/glm.hpp>

// TODO(colintan): Remove once no longer needed
#include <glm/gtx/string_cast.hpp>
#include <iostream>

namespace gfx_utils {

Mesh CreatePlane(glm::vec3 pt0, glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3) {
  Mesh mesh;

  mesh.pos_data = {pt0, pt1, pt2, pt3};

  glm::vec3 normal = glm::normalize(glm::cross(pt2 - pt1, pt0 - pt1));
  mesh.normal_data = {normal, normal, normal, normal};

  // TODO(colintan): Test that this is the correct texture coordinates
  mesh.texcoord_data = {{0.f, 1.f}, {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}};

  mesh.index_data = {0, 1, 2, 0, 2, 3};
  mesh.num_verts = 6;

  return std::move(mesh);
}

}