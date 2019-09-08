#include "gfx_utils/shader.h"

#include <fstream>
#include <sstream>

namespace gfx_utils {

bool LoadShaderSource(std::string* out_str, const std::string& path) {
  std::ifstream shader_stream(path, std::ios::in);

  if (!shader_stream.is_open()) {
    *out_str = "";
    return false;
  }

  std::stringstream str_stream;
  str_stream << shader_stream.rdbuf();
  *out_str = str_stream.str();

  shader_stream.close();

  return true;
}

}