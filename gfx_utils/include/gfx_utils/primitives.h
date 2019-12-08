#ifndef GFX_UTILS_PRIMITIVES_H_
#define GFX_UTILS_PRIMITIVES_H_

#include <glm/vec3.hpp>

#include "mesh.h"

namespace gfx_utils {

// pt0---pt3
//  | \   |
//  |  \  |
//  |   \ |
// pt1---pt2
Mesh CreatePlane(glm::vec3 pt0, glm::vec3 pt1, glm::vec3 pt2,  glm::vec3 pt3);

std::vector<Mesh> CreateRoom(float width, float height, float depth);

}

#endif