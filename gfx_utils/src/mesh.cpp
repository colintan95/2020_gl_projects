#include "gfx_utils/mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <iostream>
#include <tuple>
#include <unordered_map>

namespace gfx_utils {

bool CreateMeshFromFile(Mesh *out_mesh, const std::string& path) {
  out_mesh->pos_data.clear();
  out_mesh->normal_data.clear();
  out_mesh->texcoord_data.clear();
  out_mesh->index_data.clear();
  out_mesh->num_verts = 0; 

  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shape_data;
  std::vector<tinyobj::material_t> material_data;
  std::string warn_str, err_str;

  if (!tinyobj::LoadObj(&attribs, &shape_data, &material_data, &warn_str,
      &err_str, path.c_str())) {
    return false;
  }

  using Key = std::tuple<uint32_t, uint32_t, uint32_t>;

  struct KeyHash : public std::unary_function<Key, size_t> {
    size_t operator()(const Key & k) const {
      return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
    }
  };

  // Maps the indices from TinyObjLoader into a single index in our buffer
  std::unordered_map<Key, uint32_t, KeyHash> conversion_table;

  // Code adapted from vulkan-tutorial.com
  for (const auto& shape : shape_data) {
    for (const auto& indices : shape.mesh.indices) {
      Key key(indices.vertex_index, indices.normal_index,
              indices.texcoord_index);

      if (conversion_table.find(key) == conversion_table.end()) {
        conversion_table[key] = 
            static_cast<uint32_t>(out_mesh->pos_data.size());
        
        // TODO(colintan): Make sure that there aren't any vertices with -1's for 
        // attributes where there are valid values for other vertices
        // e.g. indices - {0, 1, -1, ...}
        if (indices.vertex_index != -1) {
          size_t index = static_cast<size_t>(indices.vertex_index) * 3;
          glm::vec3 position = {
            attribs.vertices[index], attribs.vertices[index + 1],
            attribs.vertices[index + 2]
          };
          out_mesh->pos_data.push_back(position);
        }
        if (indices.normal_index != -1) {
          size_t index = static_cast<size_t>(indices.normal_index) * 3;
          glm::vec3 normal = {
            attribs.normals[index], attribs.normals[index + 1],
            attribs.normals[index + 2]
          };
          out_mesh->normal_data.push_back(normal);
        }
        if (indices.texcoord_index != -1) {
          size_t index = static_cast<size_t>(indices.texcoord_index) * 2;
          glm::vec2 texcoord = {
            attribs.texcoords[index], attribs.texcoords[index + 1]
          };
          out_mesh->texcoord_data.push_back(texcoord);
        }
      }

      out_mesh->index_data.push_back(conversion_table[key]);
    }
  }

  out_mesh->num_verts = out_mesh->index_data.size();

  return true;
}

// TODO(colintan): Test this function
bool CreateMeshFromFileUnindexed(Mesh* out_mesh, const std::string& path) {
  out_mesh->pos_data.clear();
  out_mesh->normal_data.clear();
  out_mesh->texcoord_data.clear();
  out_mesh->index_data.clear();
  out_mesh->num_verts = 0;

  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shape_data;
  std::vector<tinyobj::material_t> material_data;
  std::string warn_str, err_str;

  if (!tinyobj::LoadObj(&attribs, &shape_data, &material_data, &warn_str,
    &err_str, path.c_str())) {
    return false;
  }

  for (const auto& shape : shape_data) {
    for (const auto& indices : shape.mesh.indices) {
      // TODO(colintan): Make sure that there aren't any vertices with -1's for 
        // attributes where there are valid values for other vertices
        // e.g. indices - {0, 1, -1, ...}
      if (indices.vertex_index != -1) {
        size_t index = static_cast<size_t>(indices.vertex_index) * 3;
        glm::vec3 position = {
          attribs.vertices[index], attribs.vertices[index + 1],
          attribs.vertices[index + 2]
        };
        out_mesh->pos_data.push_back(position);
      }
      if (indices.normal_index != -1) {
        size_t index = static_cast<size_t>(indices.normal_index) * 3;
        glm::vec3 normal = {
          attribs.normals[index], attribs.normals[index + 1],
          attribs.normals[index + 2]
        };
        out_mesh->normal_data.push_back(normal);
      }
      if (indices.texcoord_index != -1) {
        size_t index = static_cast<size_t>(indices.texcoord_index) * 2;
        glm::vec2 texcoord = {
          attribs.texcoords[index], attribs.texcoords[index + 1]
        };
        out_mesh->texcoord_data.push_back(texcoord);
      }
    }
  }

  out_mesh->num_verts = out_mesh->pos_data.size();

  return true;
}

}