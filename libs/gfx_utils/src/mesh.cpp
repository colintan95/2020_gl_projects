#include "gfx_utils/mesh.h"

#include "tinyobjloader/tiny_obj_loader.h"

#include <iostream>
#include <tuple>
#include <unordered_map>

#include "gfx_utils/texture.h"

namespace gfx_utils {

static uint64_t mesh_id_counter = 0;

Mesh::Mesh() {
  mesh_id_counter++;

  id = mesh_id_counter;
}

bool CreateMeshesFromFile(std::vector<Mesh>* out_meshes, 
                          const std::string& mtl_directory,
                          const std::string& mesh_path) {
  out_meshes->clear();

  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shape_data;
  std::vector<tinyobj::material_t> material_data;
  std::string warn_str, err_str;

  if (!tinyobj::LoadObj(&attribs, &shape_data, &material_data, &warn_str,
      &err_str, mesh_path.c_str(), mtl_directory.c_str())) {
    return false;
  }

  // Code adapted from vulkan-tutorial.com
  for (const auto& shape : shape_data) {
    Mesh out_mesh;
    out_mesh.pos_data.clear();
    out_mesh.normal_data.clear();
    out_mesh.texcoord_data.clear();
    out_mesh.index_data.clear();
    out_mesh.num_verts = 0;

    using Key = std::tuple<uint32_t, uint32_t, uint32_t>;

    struct KeyHash : public std::unary_function<Key, size_t> {
      size_t operator()(const Key& k) const {
        return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
      }
    };

    // Maps the indices from TinyObjLoader into a single index in our buffer
    std::unordered_map<Key, uint32_t, KeyHash> vert_conversion_table;

    for (const auto& indices : shape.mesh.indices) {
      Key key(indices.vertex_index, indices.normal_index,
              indices.texcoord_index);

      if (vert_conversion_table.find(key) == vert_conversion_table.end()) {
        vert_conversion_table[key] = 
            static_cast<uint32_t>(out_mesh.pos_data.size());
        
        // TODO(colintan): Make sure that there aren't any vertices with -1's for 
        // attributes where there are valid values for other vertices
        // e.g. indices - {0, 1, -1, ...}
        if (indices.vertex_index != -1) {
          size_t index = static_cast<size_t>(indices.vertex_index) * 3;
          glm::vec3 position = {
            attribs.vertices[index], attribs.vertices[index + 1],
            attribs.vertices[index + 2]
          };
          out_mesh.pos_data.push_back(position);
        }
        if (indices.normal_index != -1) {
          size_t index = static_cast<size_t>(indices.normal_index) * 3;
          glm::vec3 normal = {
            attribs.normals[index], attribs.normals[index + 1],
            attribs.normals[index + 2]
          };
          out_mesh.normal_data.push_back(normal);
        }
        if (indices.texcoord_index != -1) {
          size_t index = static_cast<size_t>(indices.texcoord_index) * 2;
          glm::vec2 texcoord = {
            attribs.texcoords[index], attribs.texcoords[index + 1]
          };
          out_mesh.texcoord_data.push_back(texcoord);
        }
      }

      out_mesh.index_data.push_back(vert_conversion_table[key]);
    }

    std::unordered_map<int, unsigned int> mtl_conversion_table;

    // Adds the material data into the mesh
    for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); ++i) {
      int loader_id = shape.mesh.material_ids[i];

      if (loader_id == -1) {
        continue;
      }
      
      if (mtl_conversion_table.find(loader_id) == mtl_conversion_table.end()) {
        tinyobj::material_t loader_mtl = material_data[loader_id];
        Material mtl;

        mtl.ambient_color  = glm::vec3(loader_mtl.ambient[0],
                                       loader_mtl.ambient[1],
                                       loader_mtl.ambient[2]);
        mtl.diffuse_color  = glm::vec3(loader_mtl.diffuse[0],
                                       loader_mtl.diffuse[1],
                                       loader_mtl.diffuse[2]);
        mtl.specular_color = glm::vec3(loader_mtl.specular[0],
                                       loader_mtl.specular[1],
                                       loader_mtl.specular[2]);
        mtl.emission_color = glm::vec3(loader_mtl.emission[0],
                                      loader_mtl.emission[1],
                                      loader_mtl.emission[2]);
        mtl.shininess = loader_mtl.shininess;

        switch (loader_mtl.illum) {
        case 0:
          mtl.illum = kIllumModelColorOnly;
          break;
        case 1:
          mtl.illum = kIllumModelAmbientOnly;
          break;
        case 2:
          mtl.illum = kIllumModelHighlight;
          break;
        default:
          mtl.illum = kIllumModelInvalid;
          break;
        }

        if (!loader_mtl.ambient_texname.empty()) {
          mtl.ambient_texname = loader_mtl.ambient_texname;
        }
        if (!loader_mtl.diffuse_texname.empty()) {
          mtl.diffuse_texname = loader_mtl.diffuse_texname;
        }
        if (!loader_mtl.specular_texname.empty()) {
          mtl.specular_texname = loader_mtl.specular_texname;
        }

        mtl_conversion_table[loader_id] = out_mesh.material_list.size();
        out_mesh.material_list.push_back(mtl);
      }

      unsigned int mtl_id = mtl_conversion_table[loader_id];

      for (size_t j = 0; j < shape.mesh.num_face_vertices[i]; ++j) {
        out_mesh.mtl_id_data.push_back(mtl_id);
      }
    }

    // Checks that all faces are triangles
    for (unsigned int num_verts : shape.mesh.num_face_vertices) {
      if (num_verts != 3) {
        std::cerr << "Face is not a triangle. Failing." << std::endl;
        out_meshes->clear();
        return false;
      }
    }

    out_mesh.num_verts = out_mesh.index_data.size();

    out_meshes->push_back(out_mesh);
  }

  return true;
}
//
//// TODO(colintan): Test this function
//bool CreateMeshFromFileUnindexed(Mesh* out_mesh, const std::string& path) {
//  out_mesh->pos_data.clear();
//  out_mesh->normal_data.clear();
//  out_mesh->texcoord_data.clear();
//  out_mesh->index_data.clear();
//  out_mesh->num_verts = 0;
//
//  tinyobj::attrib_t attribs;
//  std::vector<tinyobj::shape_t> shape_data;
//  std::vector<tinyobj::material_t> material_data;
//  std::string warn_str, err_str;
//
//  if (!tinyobj::LoadObj(&attribs, &shape_data, &material_data, &warn_str,
//    &err_str, path.c_str())) {
//    return false;
//  }
//
//  for (const auto& shape : shape_data) {
//    for (const auto& indices : shape.mesh.indices) {
//      // TODO(colintan): Make sure that there aren't any vertices with -1's for 
//        // attributes where there are valid values for other vertices
//        // e.g. indices - {0, 1, -1, ...}
//      if (indices.vertex_index != -1) {
//        size_t index = static_cast<size_t>(indices.vertex_index) * 3;
//        glm::vec3 position = {
//          attribs.vertices[index], attribs.vertices[index + 1],
//          attribs.vertices[index + 2]
//        };
//        out_mesh->pos_data.push_back(position);
//      }
//      if (indices.normal_index != -1) {
//        size_t index = static_cast<size_t>(indices.normal_index) * 3;
//        glm::vec3 normal = {
//          attribs.normals[index], attribs.normals[index + 1],
//          attribs.normals[index + 2]
//        };
//        out_mesh->normal_data.push_back(normal);
//      }
//      if (indices.texcoord_index != -1) {
//        size_t index = static_cast<size_t>(indices.texcoord_index) * 2;
//        glm::vec2 texcoord = {
//          attribs.texcoords[index], attribs.texcoords[index + 1]
//        };
//        out_mesh->texcoord_data.push_back(texcoord);
//      }
//    }
//  }
//
//  out_mesh->num_verts = out_mesh->pos_data.size();
//
//  return true;
//}

void ClearMesh(Mesh *mesh) {
  mesh->pos_data.clear();
  mesh->normal_data.clear();
  mesh->texcoord_data.clear();

  mesh->index_data.clear();

  mesh->mtl_id_data.clear();

  mesh->num_verts = 0;

  mesh->material_list;
}

}