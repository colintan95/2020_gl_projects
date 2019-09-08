#ifndef GFX_UTILS_SHADER_H_
#define GFX_UTILS_SHADER_H_

#include <string>

namespace gfx_utils {

bool LoadShaderSource(std::string* out_str, const std::string& path);

}

#endif