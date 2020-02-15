#include "gfx_utils/scene/scene.h"

#include "nlohmann/json.hpp"

#include "tinyobjloader/tiny_obj_loader.h"

#include <iostream>
#include <fstream>

#include "gfx_utils/texture.h"
#include "gfx_utils/scene/data_source.h"
#include "gfx_utils/scene/light_loader.h"

using json = nlohmann::json;

namespace gfx_utils {

bool Scene::LoadSceneFromJson(const std::string& path) {
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

    bool indexed = true;

    auto indexed_it = model_prop.find("indexed");
    if (indexed_it != model_prop.end()) {
      indexed = model_prop["indexed"];
      std::cout << "Indexed: " << indexed << std::endl;
    }

    auto model_ptr = model_loader_.LoadModelFromFile(
        name, mtl_dir, file, indexed);

    if (!model_ptr) {
      std::cerr << "Could not load model: " << name;
      continue;
    }

    models_[name] = model_ptr;
    models_list_.push_back(model_ptr);

    // Load the textures for the meshes in the model

    for (gfx_utils::Mesh mesh : model_ptr->GetMeshes()) {
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

          auto tex_ptr = std::make_shared<Texture>();
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

    const std::string& entity_name = entity_prop["name"];

    auto entity = std::make_shared<Entity>(entity_name);
    
    entity->SetModel(model_it->second);

    // Set location if present
    if (entity_prop.find("position") != entity_prop.end()) {
      auto pos_prop = entity_prop["position"];
      entity->SetLocation(glm::vec3(pos_prop[0].get<float>(),
                                    pos_prop[1].get<float>(),
                                    pos_prop[2].get<float>()));
    }

    // Set scale if present
    if (entity_prop.find("scale") != entity_prop.end()) {
      auto scale_prop = entity_prop["scale"];
      entity->SetScale(glm::vec3(scale_prop[0].get<float>(),
                                 scale_prop[1].get<float>(),
                                 scale_prop[2].get<float>()));
    }
    
    entities_[entity_name] = entity;
    entities_list_.push_back(entity);
  }

  // Load the lights

  auto lights_it = json_obj.find("lights");
  if (lights_it == json_obj.end()) {
    std::cerr << "Could not find 'lights' json property" << std::endl;
    return false;
  }

  auto lights_array = lights_it.value();
  for (auto it = lights_array.begin(); it != lights_array.end(); ++it) {
    DataSource data_src;
    
    auto light_prop = it.value();
    for (auto& prop : light_prop.items()) {
      DataEntry entry;

      auto key = prop.key();
      auto val = prop.value();

      switch (val.type()) {
      case json::value_t::array:
        if (val.size() == 3) {
          entry.val_vec3 = glm::vec3(val[0].get<float>(),
                                     val[1].get<float>(),
                                     val[2].get<float>());
        }
        else {
          std::cerr << "Type not supported" << std::endl;
        }
        break;
      case json::value_t::string:
        entry.val_str = val.get<std::string>();
        break;
      case json::value_t::number_float:
        entry.val_float = val.get<float>();
        break;
      default:
        std::cerr << "Type not supported" << std::endl;
      }

      data_src.data_[key] = std::move(entry);
    }

    const std::string& type = light_prop["type"];
    LightPtr light_ptr = (*kLightLoadFuncTable.at(type))(data_src);

    lights_[light_prop["name"]] = light_ptr;
    
    LightListEntry entry;
    entry.type = type;
    entry.ptr = light_ptr;
    lights_list_.push_back(std::move(entry));
  }

  return true;
}

void Scene::AddEntity(EntityPtr entity) {
  if (!entity->HasModel()) {
    std::cerr << "Entity does not have a model." << std::endl;
    return;
  }

  auto entity_name = entity->GetName();
  auto model_name = entity->GetModel()->GetName();

  if (entities_.find(entity_name) != entities_.end()) {
    std::cerr << "Entity name already used." << std::endl;
    return;
  }

  if (models_.find(model_name) != models_.end()) {
    std::cerr << "Model name already used." << std::endl;
    return;
  }

  entities_[entity_name] = entity;
  entities_list_.push_back(entity);

  models_[model_name] = entity->GetModel();
  models_list_.push_back(entity->GetModel());
}

const ModelList& Scene::GetModels() {
  return models_list_;
}
  
const EntityList& Scene::GetEntities() {
  return entities_list_;
}

const TextureNameMap& Scene::GetTextureNameMap() {
  return textures_;
}

ModelPtr Scene::GetModel(const std::string& name) {
  auto it = models_.find(name);
  if (it == models_.end()) {
    return nullptr;
  }
  return it->second;
}

EntityPtr Scene::GetEntity(const std::string& name) {
  auto it = entities_.find(name);
  if (it == entities_.end()) {
    return nullptr;
  }
  return it->second;
}

TexturePtr Scene::GetTexture(const std::string& name) {
  auto it = textures_.find(name);
  if (it == textures_.end()) {
    return nullptr;
  }
  return it->second;
}

} // namespace gfx_utils