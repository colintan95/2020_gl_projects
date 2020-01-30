#ifndef GFX_UTILS_SCENE_LIGHT_LOADER_H_
#define GFX_UTILS_SCENE_LIGHT_LOADER_H_

#include <memory>
#include <unordered_map>
#include <string>

#include "gfx_utils/lights.h"
#include "gfx_utils/scene/data_source.h"

namespace gfx_utils {

std::shared_ptr<Light> LoadPointLight(const DataSource& data_src);
std::shared_ptr<Light> LoadSpotlight(const DataSource& data_src);

// TODO(colintan): Do this more elegantly
const std::unordered_map<std::string, 
                         std::shared_ptr<Light> (*)(const DataSource&)>
    kLightLoadFuncTable = {
      { "point_light", LoadPointLight },
      { "spotlight",   LoadSpotlight }
    };

} // namespace gfx_utils

#endif // GFX_UTILS_SCENE_LIGHT_LOADER_H_