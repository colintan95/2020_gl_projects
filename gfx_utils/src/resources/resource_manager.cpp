#include "gfx_utils/resources/resource_manager.h"

#include "nlohmann/json.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#include <iostream>
#include <fstream>

#include "gfx_utils/texture.h"

using json = nlohmann::json;

namespace gfx_utils {

bool ResourceManager::LoadResourcesFromJson(const std::string& path) {
  std::ifstream json_fs(path);
  if (!json_fs.is_open()) {
    std::cerr << "Could not open file: " << path << std::endl;
    return false;
  }
  json json_obj = json::parse(json_fs);
  json_fs.close();

  // Load the models

  auto models_it = json_obj.find("models");
  if (models_it == json_obj.end()) {
    std::cerr << "Could not find 'models' json property" << std::endl;
    return false;
  }

  auto models_array = models_it.value();
  for (auto it = models_array.begin(); it != models_array.end(); ++it) {
    auto model_prop = it.value();

    const std::string& name = model_prop["name"];
    const std::string& mtl_dir = model_prop["mtl_dir"];
    const std::string& file = model_prop["file"];

    if (!LoadModelFromFile(name, mtl_dir, file)) {
      std::cerr << "Could not load model: " << model_prop["name"];
      continue;
    }

    auto model_ptr = models_[name];

    // Load the textures for the meshes in the model

    for (gfx_utils::Mesh mesh : model_ptr->meshes) {
      for (gfx_utils::Material mtl : mesh.material_list) {
        // TODO(colintan): Store the texture strings as a collection in the
        // material so that this code works even when we add or remove textures
        // from the Material struct
        std::string texnames[] = {
          mtl.ambient_texname, mtl.diffuse_texname, mtl.specular_texname
        };

        for (const auto& texname : texnames) {
          // Blank texname means there isn't a texture
          if (texname.empty()) {
            continue;
          }

          // TODO(colintan): Possibly set this during the loading of the model
          mesh.is_textured = true;

          // Already loaded the texture
          if (textures_.find(texname) != textures_.end()) {
            continue;
          }

          std::shared_ptr<Texture> tex_ptr = std::make_shared<Texture>();
          if (!CreateTextureFromFile(tex_ptr.get(), mtl_dir, texname)) {
            std::cerr << "Failed to load texture: " << texname << std::endl;
            // TODO(colintan): Do better error handling here
            continue;
          }
          textures_[texname] = tex_ptr;
        }
      }
    }
  }

  // Load the entities

  auto entities_it = json_obj.find("entities");

  if (entities_it == json_obj.end()) {
    std::cerr << "Could not find 'entities' json property" << std::endl;
    return false;
  }

  auto entities_array = entities_it.value();

  for (auto it = entities_array.begin(); it != entities_array.end(); ++it) {
    auto entity_prop = it.value();
    const std::string& model_name = entity_prop["model"];

    auto model_it = models_.find(model_name);
    if (model_it == models_.end()) {
      std::cerr << "Could not find model: " << model_name << std::endl;
      continue;
    }

    auto entity = std::make_shared<Entity>();
    entity->SetModel(model_it->second);
    
    entities_[entity_prop["name"]] = entity;
    entities_list_.push_back(entity);
  }

  return true;
}

bool ResourceManager::LoadModelFromFile(const std::string& name,
                                        const std::string& mtl_directory,
                                        const std::string& path) {
  std::shared_ptr<Model> model_ptr = std::make_shared<Model>();

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
        return false;
      }
    }

    out_mesh.num_verts = out_mesh.index_data.size();

    model_ptr->meshes.push_back(std::move(out_mesh));
    models_[name] = model_ptr;
    models_list_.push_back(model_ptr);
  }

  return true;
}

const ModelList& ResourceManager::GetModels() {
  return models_list_;
}
  
const EntityList& ResourceManager::GetEntities() {
  return entities_list_;
}

const TextureNameMap& ResourceManager::GetTextureNameMap() {
  return textures_;
}

ModelPtr ResourceManager::GetModel(const std::string& name) {
  auto it = models_.find(name);
  if (it == models_.end()) {
    return nullptr;
  }
  return it->second;
}

EntityPtr ResourceManager::GetEntity(const std::string& name) {
  auto it = entities_.find(name);
  if (it == entities_.end()) {
    return nullptr;
  }
  return it->second;
}

TexturePtr ResourceManager::GetTexture(const std::string& name) {
  auto it = textures_.find(name);
  if (it == textures_.end()) {
    return nullptr;
  }
  return it->second;
}

} // namespace gfx_utils