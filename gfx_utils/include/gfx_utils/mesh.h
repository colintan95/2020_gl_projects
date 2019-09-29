#ifndef GFX_UTILS_MESH_
#define GFX_UTILS_MESH_

#include <vector>
#include <string>
#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "material.h"

namespace gfx_utils {

struct Mesh {
  std::vector<glm::vec3> pos_data;
  std::vector<glm::vec3> normal_data;
  std::vector<glm::vec2> texcoord_data;

  std::vector<uint32_t> index_data;

  std::vector<uint32_t> mtl_id_data; // indexes into the material list

  uint32_t num_verts;

  std::vector<Material> material_list;
};

bool CreateMeshesFromFile(std::vector<Mesh> *out_meshes, const std::string& path);

//bool CreateMeshFromFileUnindexed(Mesh *out_mesh, const std::string& path);

}

#endif