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

std::vector<Mesh> CreateRoom(float width, float height, float depth) {
  float half_width = width / 2.f;
  float half_height = height / 2.f;
  float half_depth = depth / 2.f;

  glm::vec3 points[] = {{ half_width,  half_height,  half_depth},
                        {-half_width,  half_height,  half_depth},
                        { half_width, -half_height,  half_depth},
                        {-half_width, -half_height,  half_depth},
                        { half_width,  half_height, -half_depth},
                        {-half_width,  half_height, -half_depth},
                        { half_width, -half_height, -half_depth},
                        {-half_width, -half_height, -half_depth}};

  std::vector<Mesh> meshes;
  // Sides of the room (in anticlockwise order)
  meshes.push_back(CreatePlane(points[0], points[2], points[3], points[1]));
  meshes.push_back(CreatePlane(points[4], points[6], points[2], points[0]));
  meshes.push_back(CreatePlane(points[5], points[7], points[6], points[4]));
  meshes.push_back(CreatePlane(points[1], points[3], points[7], points[5]));
  // Top and bottom of the room
  meshes.push_back(CreatePlane(points[1], points[5], points[4], points[0]));
  meshes.push_back(CreatePlane(points[7], points[3], points[2], points[6]));

  return std::move(meshes);
}

}