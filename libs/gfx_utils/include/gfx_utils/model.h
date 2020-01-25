#ifndef GFX_UTILS_MODEL_
#define GFX_UTILS_MODEL_

#include <string>
#include <vector>

#include "mesh.h"

#include <glm/glm.hpp>

namespace gfx_utils {

class Model {
public:
  Model(const std::string& name) {
    name_ = name;
  }

  std::vector<Mesh>& GetMeshes() {
    return meshes_;
  }

  const std::string& GetName() const {
    return name_;
  }

private:
  std::string name_;

  std::vector<Mesh> meshes_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_MODEL_H_