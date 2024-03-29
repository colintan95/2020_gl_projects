#ifndef GFX_UTILS_SCENE_SCENE_H_
#define GFX_UTILS_SCENE_SCENE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "model_loader.h"

#include "gfx_utils/lights.h"
#include "gfx_utils/model.h"
#include "gfx_utils/entity.h"
#include "gfx_utils/texture.h"

namespace gfx_utils {

using ModelPtr = std::shared_ptr<Model>;
using EntityPtr = std::shared_ptr<Entity>;
using TexturePtr = std::shared_ptr<Texture>;
using CubemapPtr = std::shared_ptr<Cubemap>;
using LightPtr = std::shared_ptr<Light>;

using ModelList = std::vector<ModelPtr>;
using EntityList = std::vector<EntityPtr>;

template <typename T>
using LightList = std::vector<std::shared_ptr<T>>;

using ModelNameMap = std::unordered_map<std::string, ModelPtr>;
using EntityNameMap = std::unordered_map<std::string, EntityPtr>;
using TextureNameMap = std::unordered_map<std::string, TexturePtr>;
using CubemapNameMap = std::unordered_map<std::string, CubemapPtr>;
using LightNameMap = std::unordered_map<std::string, LightPtr>;

class Scene {
public:
  bool LoadSceneFromJson(const std::string& path);

  void AddEntity(EntityPtr entity);

  const ModelList& GetModels();
  const EntityList& GetEntities();

  // Returns only lights of type T
  template<typename T>
  LightList<T> GetLightsByType();

  const TextureNameMap& GetTextureNameMap();
  const CubemapNameMap& GetCubemapNameMap();

  ModelPtr GetModel(const std::string& name);
  EntityPtr GetEntity(const std::string& name);
  TexturePtr GetTexture(const std::string& name);
  CubemapPtr GetCubemap(const std::string& name);

private:
  struct LightListEntry {
    std::string type;
    LightPtr ptr;
  };

private:
  ModelNameMap models_;
  EntityNameMap entities_;    
  TextureNameMap textures_;
  CubemapNameMap cubemaps_;
  LightNameMap lights_;

  ModelList models_list_;
  EntityList entities_list_;
  std::vector<LightListEntry> lights_list_;

  ModelLoader model_loader_;
};

template<typename T>
LightList<T> Scene::GetLightsByType() {
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

#endif // GFX_UTILS_SCENE_SCENE_H_