#ifndef GFX_UTILS_RESOURCE_RESOURCE_MANAGER_H_
#define GFX_UTILS_RESOURCE_RESOURCE_MANAGER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "gfx_utils/lights.h"
#include "gfx_utils/model.h"
#include "gfx_utils/entity.h"
#include "gfx_utils/texture.h"

namespace gfx_utils {

using ModelPtr = std::shared_ptr<Model>;
using EntityPtr = std::shared_ptr<Entity>;
using TexturePtr = std::shared_ptr<Texture>;
using LightPtr = std::shared_ptr<Light>;

using ModelList = std::vector<ModelPtr>;
using EntityList = std::vector<EntityPtr>;

template <typename T>
using LightList = std::vector<std::shared_ptr<T>>;

using ModelNameMap = std::unordered_map<std::string, ModelPtr>;
using EntityNameMap = std::unordered_map<std::string, EntityPtr>;
using TextureNameMap = std::unordered_map<std::string, TexturePtr>;
using LightNameMap = std::unordered_map<std::string, LightPtr>;

class ResourceManager {
public:
  bool LoadResourcesFromJson(const std::string& path);

  bool LoadModelFromFile(const std::string& name,
                          const std::string& mtl_directory,
                          const std::string& path);

  const ModelList& GetModels();
  const EntityList& GetEntities();

  // Returns only lights of type T
  template<typename T>
  LightList<T> GetLightsByType();

  const TextureNameMap& GetTextureNameMap();

  ModelPtr GetModel(const std::string& name);
  EntityPtr GetEntity(const std::string& name);
  TexturePtr GetTexture(const std::string& name);

private:
  struct LightListEntry {
    std::string type;
    LightPtr ptr;
  };

private:
  ModelNameMap models_;
  EntityNameMap entities_;    
  TextureNameMap textures_;
  LightNameMap lights_;

  ModelList models_list_;
  EntityList entities_list_;
  std::vector<LightListEntry> lights_list_;
};

template<typename T>
LightList<T> ResourceManager::GetLightsByType() {
  LightList<T> result_list;

  const char* type = T::GetType();
  for (const auto& entry : lights_list_) {
    if (entry.type == type) {
      std::shared_ptr<T> derived_ptr = std::static_pointer_cast<T>(entry.ptr);
      result_list.push_back(derived_ptr);
    }
  }

  return result_list;
}

} // namespace gfx_utils

#endif // GFX_UTILS_RESOURCE_RESOURCE_MANAGER_H_