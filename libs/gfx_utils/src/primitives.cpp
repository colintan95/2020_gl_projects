#include "gfx_utils/primitives.h"

#include <glm/glm.hpp>

// TODO(colintan): Remove once no longer needed
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <cmath>

#include "gfx_utils/material.h"

namespace gfx_utils {

static Material CreateDefaultMaterial() {
  Material mtl;

  mtl.ambient_color = glm::vec3(1.f, 1.f, 1.f);
  mtl.diffuse_color = glm::vec3(1.f, 1.f, 1.f);
  mtl.specular_color = glm::vec3(0.2f, 0.2f, 0.2f);
  mtl.emission_color = glm::vec3(0.f, 0.f, 0.f);

  mtl.shininess = 10.f;

  mtl.illum = kIllumModelHighlight;

  mtl.ambient_texname = "";
  mtl.diffuse_texname = "";
  mtl.specular_texname = "";
  
  return std::move(mtl);
}

Mesh CreatePlane(glm::vec3 pt0, glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3) {
  Mesh mesh;

  mesh.pos_data = {pt0, pt1, pt2, pt3};

  glm::vec3 normal = glm::normalize(glm::cross(pt2 - pt1, pt0 - pt1));
  mesh.normal_data = {normal, normal, normal, normal};

  // TODO(colintan): Test that this is the correct texture coordinates
  mesh.texcoord_data = {{0.f, 1.f}, {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}};

  mesh.mtl_id_data = {0, 0, 0, 0};
  mesh.material_list.push_back(std::move(CreateDefaultMaterial()));

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