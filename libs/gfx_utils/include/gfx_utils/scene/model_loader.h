#ifndef GFX_UTILS_SCENE_MODEL_LOADER_H_
#define GFX_UTILS_SCENE_MODEL_LOADER_H_

#include <string>
#include <memory>

#include "gfx_utils/model.h"

namespace gfx_utils {

class ModelLoader {
public:
  std::shared_ptr<Model> LoadModelFromFile(const std::string& name,
                                           const std::string& mtl_directory,
                                           const std::string& path);
};

} // namespace gfx_utils

#endif // GFX_UTILS_SCENE_MODEL_LOADER_H_