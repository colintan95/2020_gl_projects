#include "gfx_utils/scene/model_loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

namespace gfx_utils {

std::shared_ptr<Model> ModelLoader::LoadModelFromFile(
    const std::string& name, 
    const std::string& mtl_directory,
    const std::string& path,
    bool indexed) 
{
  auto model_ptr = std::make_shared<Model>(name);

  tinyobj::attrib_t attribs;
  std::vector<tinyobj::shape_t> shape_data;
  std::vector<tinyobj::material_t> material_data;
  std::string warn_str, err_str;

  if (!tinyobj::LoadObj(&attribs, &shape_data, &material_data, &warn_str,
      &err_str, path.c_str(), mtl_directory.c_str())) {
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

    if (indexed) {
      LoadVertexData(&out_mesh, shape, attribs);
    }
    else {
      LoadVertexDataNoIndex(&out_mesh, shape, attribs);
    }

    LoadMaterialData(&out_mesh, shape, material_data);

    model_ptr->GetMeshes().push_back(std::move(out_mesh));
  }

  return model_ptr;
}

void ModelLoader::LoadVertexData(Mesh* mesh, 
                                 const tinyobj::shape_t& shape,
                                 const tinyobj::attrib_t& attribs) {
  using Key = std::tuple<uint32_t, uint32_t, uint32_t>;

  struct KeyHash : public std::unary_function<Key, size_t> {
    size_t operator()(const Key& k) const {
      return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
    }
  };      

  // Maps the indices from TinyObjLoader into a single index in our buffer
  std::unordered_map<Key, uint32_t, KeyHash> vert_conversion_table;

  // std::cout << "No. of indices: " << shape.mesh.indices.size() << std::endl;
  // std::cout << "No. of faces: " << shape.mesh.num_face_vertices.size() << std::endl;

  for (const auto& indices : shape.mesh.indices) {
    Key key(indices.vertex_index, indices.normal_index,
            indices.texcoord_index);

    if (vert_conversion_table.find(key) == vert_conversion_table.end()) {
      vert_conversion_table[key] = 
          static_cast<uint32_t>(mesh->pos_data.size());
      
      // TODO(colintan): Make sure that there aren't any vertices with -1's for 
      // attributes where there are valid values for other vertices
      // e.g. indices - {0, 1, -1, ...}
      if (indices.vertex_index != -1) {
        size_t index = static_cast<size_t>(indices.vertex_index) * 3;
        glm::vec3 position = {
          attribs.vertices[index], attribs.vertices[index + 1],
          attribs.vertices[index + 2]
        };
        mesh->pos_data.push_back(position);
      }

      if (indices.normal_index != -1) {
        size_t index = static_cast<size_t>(indices.normal_index) * 3;
        glm::vec3 normal = {
          attribs.normals[index], attribs.normals[index + 1],
          attribs.normals[index + 2]
        };
        mesh->normal_data.push_back(normal);
      }

      if (indices.texcoord_index != -1) {
        size_t index = static_cast<size_t>(indices.texcoord_index) * 2;
        glm::vec2 texcoord = {
          attribs.texcoords[index], attribs.texcoords[index + 1]
        };
        mesh->texcoord_data.push_back(texcoord);
      }
    }

    mesh->index_data.push_back(vert_conversion_table[key]); 
  }         

  mesh->num_verts = static_cast<uint32_t>(mesh->index_data.size());
}

bool ModelLoader::LoadVertexDataNoIndex(Mesh* mesh, 
                                        const tinyobj::shape_t& shape,
                                        const tinyobj::attrib_t& attribs) {
  // Some checks that the data is in a format we support

  for (unsigned int num_verts : shape.mesh.num_face_vertices) {
    if (num_verts != 3) {
      std::cerr << "Face is not a triangle. Failing." << std::endl;
      return false;
    }
  }

  // For each kind of  vertex data (e.g. pos, normal) either there is data for 
  // all vertices, or no data at all
  size_t num_verts = shape.mesh.num_face_vertices.size() * 3;

  if (num_verts == 0) {
    std::cerr << "Mesh has no vertex data." << std::endl;
    return false;
  }

  bool has_pos_data      = shape.mesh.indices[0].vertex_index != -1;
  bool has_normal_data   = shape.mesh.indices[0].normal_index != -1;
  bool has_texcoord_data = shape.mesh.indices[0].texcoord_index != -1;

  for (auto vert_indices : shape.mesh.indices) {
    if (has_pos_data && attribs.vertices[vert_indices.vertex_index] == -1) {
      return false;
    }
    else if (!has_pos_data && attribs.vertices[vert_indices.vertex_index] != -1) {
      return false;
    }

    // TODO(colintan): Add checks for the other types of vertex data
  }

  size_t indices_idx = 0;

  for (unsigned char num_vert : shape.mesh.num_face_vertices) {

    for (unsigned char i = 0; i < num_vert; ++i) {
      auto vert_indices = shape.mesh.indices[indices_idx + i];

      if (vert_indices.vertex_index != -1) {
        mesh->pos_data.push_back(
            GetPositionAtIndex(vert_indices.vertex_index, attribs));
      }

      if (vert_indices.normal_index != -1) {
        mesh->normal_data.push_back(
            GetNormalAtIndex(vert_indices.normal_index, attribs));
      }
      else {
        // Compute normals if .obj file doesn't have them

        size_t curr_indices_idx = indices_idx + i;
        size_t prev_indices_idx = indices_idx + (i > 0 ? i-1 : num_vert-1);
        size_t next_indices_idx = indices_idx + (i < num_vert-1 ? i+1 : 0);

        size_t curr_vert_idx = 
            shape.mesh.indices[curr_indices_idx].vertex_index;
        size_t prev_vert_idx = 
            shape.mesh.indices[prev_indices_idx].vertex_index;
        size_t next_vert_idx = 
            shape.mesh.indices[next_indices_idx].vertex_index;

        glm::vec3 curr_pos = GetPositionAtIndex(curr_vert_idx, attribs);
        glm::vec3 prev_pos = GetPositionAtIndex(prev_vert_idx, attribs);
        glm::vec3 next_pos = GetPositionAtIndex(next_vert_idx, attribs);

        glm::vec3 normal = glm::normalize(
            glm::cross(prev_pos - curr_pos, next_pos - curr_pos));

        mesh->normal_data.push_back(normal);
      }

      if (vert_indices.texcoord_index != -1) {
        mesh->texcoord_data.push_back(
            GetTexcoordAtIndex(vert_indices.texcoord_index, attribs));
      }
    }

    indices_idx += num_vert;
  } 

  mesh->index_data.clear();

  mesh->num_verts = static_cast<uint32_t>(mesh->pos_data.size());

  return true;
}

void ModelLoader::LoadMaterialData(
    Mesh* mesh,
    const tinyobj::shape_t& shape,
    const std::vector<tinyobj::material_t>& material_data) 
{
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

      mtl_conversion_table[loader_id] = 
        static_cast<uint32_t>(mesh->material_list.size());
      mesh->material_list.push_back(mtl);
    }

    unsigned int mtl_id = mtl_conversion_table[loader_id];

    for (size_t j = 0; j < shape.mesh.num_face_vertices[i]; ++j) {
      mesh->mtl_id_data.push_back(mtl_id);
    }
  }
}

glm::vec3 ModelLoader::GetPositionAtIndex(
    size_t vert_idx, const tinyobj::attrib_t& attribs
) {
  size_t buf_idx = vert_idx * 3;

  glm::vec3 position = {
    attribs.vertices[buf_idx + 0],
    attribs.vertices[buf_idx + 1],
    attribs.vertices[buf_idx + 2]
  };

  return position;
}

glm::vec3 ModelLoader::GetNormalAtIndex(
    size_t vert_idx, const tinyobj::attrib_t& attribs
) {
  size_t buf_idx = vert_idx * 3;

  glm::vec3 normal = {
    attribs.normals[buf_idx + 0],
    attribs.normals[buf_idx + 1],
    attribs.normals[buf_idx + 2]
  };

  return normal;
}

glm::vec2 ModelLoader::GetTexcoordAtIndex(
    size_t vert_idx, const tinyobj::attrib_t& attribs
) {
  size_t buf_idx = vert_idx * 2;

  glm::vec2 texcoord = {
    attribs.texcoords[buf_idx + 0],
    attribs.texcoords[buf_idx + 1]
  };

  return texcoord;
}

} // namespace gfx_utils                         