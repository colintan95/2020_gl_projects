#include "gfx_utils/primitives.h"

#include <glm/glm.hpp>

// TODO(colintan): Remove once no longer needed
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <cmath>

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

Mesh CreatePerspectiveFrustum(float fov, float aspect_ratio, float near_plane,
                              float far_plane) {
  float near_half_height = near_plane * tan(fov / 2.f);
  float near_half_width = near_half_height * aspect_ratio;

  float far_half_height = far_plane * tan(fov / 2.f); 
  float far_half_width = far_half_height * aspect_ratio;

  Mesh mesh;

  mesh.pos_data = 
      { glm::vec3(-near_half_width,  near_half_height, -near_plane),
        glm::vec3(-near_half_width, -near_half_height, -near_plane), 
        glm::vec3( near_half_width, -near_half_height, -near_plane), 
        glm::vec3( near_half_width,  near_half_height, -near_plane), 
        glm::vec3(-far_half_width,  far_half_height, -far_plane),
        glm::vec3(-far_half_width, -far_half_height, -far_plane), 
        glm::vec3( far_half_width, -far_half_height, -far_plane), 
        glm::vec3( far_half_width,  far_half_height, -far_plane) };

  mesh.index_data = { 0, 1, 2, 0, 2, 3, // near plane
                      4, 5, 6, 4, 6, 7, // far plane
                      4, 5, 1, 4, 1, 0, // left plane
                      3, 2, 6, 3, 6, 7, // right plane
                      4, 0, 3, 4, 3, 7, // top plane
                      1, 5, 6, 1, 6, 2 // bottom plane 
                     };    
  mesh.num_verts = static_cast<uint32_t>(mesh.index_data.size());   

  return std::move(mesh);                                                         
}

}