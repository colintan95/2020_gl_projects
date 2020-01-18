#ifndef GFX_UTILS_MODEL_
#define GFX_UTILS_MODEL_

#include <vector>

#include "mesh.h"

namespace gfx_utils {

struct Model {
  std::vector<Mesh> meshes;
};

} // namespace gfx_utils

#endif // GFX_UTILS_MODEL_H_