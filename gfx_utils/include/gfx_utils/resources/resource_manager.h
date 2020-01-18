#ifndef GFX_UTILS_RESOURCE_MANAGER_H_
#define GFX_UTILS_RESOURCE_MANAGER_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "gfx_utils/model.h"
#include "gfx_utils/entity.h"
#include "gfx_utils/texture.h"

namespace gfx_utils {

using ModelPtr = std::shared_ptr<Model>;
using EntityPtr = std::shared_ptr<Entity>;
using TexturePtr = std::shared_ptr<Texture>;

using ModelList = std::vector<ModelPtr>;
using EntityList = std::vector<EntityPtr>;

using ModelNameMap = std::unordered_map<std::string, ModelPtr>;
using EntityNameMap = std::unordered_map<std::string, EntityPtr>;
using TextureNameMap = std::unordered_map<std::string, TexturePtr>;

class ResourceManager {
public:
  bool LoadResourcesFromJson(const std::string& path);

  bool LoadModelFromFile(const std::string& name,
                          const std::string& mtl_directory,
                          const std::string& path);

  const ModelList& GetModels();
  const EntityList& GetEntities();

  const TextureNameMap& GetTextureNameMap();

  ModelPtr GetModel(const std::string& name);
  EntityPtr GetEntity(const std::string& name);
  TexturePtr GetTexture(const std::string& name);

private:
  ModelNameMap models_;
  EntityNameMap entities_;    
  TextureNameMap textures_;

  ModelList models_list_;
  EntityList entities_list_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_RESOURCE_MANAGER_H_