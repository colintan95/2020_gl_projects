#ifndef GFX_UTILS_RESOURCE_DATA_SOURCE_H_
#define GFX_UTILS_RESOURCE_DATA_SOURCE_H_

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace gfx_utils {

// TODO(colintan): Think of a better way to do this
struct DataEntry {
  std::string val_str;
  glm::vec3 val_vec3;
  glm::vec4 val_vec4;
  float val_float;
};

class DataSource {
public:
  DataEntry GetEntry(const std::string& name) const;

private:
  // Entry: (name, val)
  std::unordered_map<std::string, DataEntry> data_;

private:
  friend class ResourceManager;
};

} // namespace gfx_utils

#endif // GFX_UTILS_RESOURCE_DATA_SOURCE_H_