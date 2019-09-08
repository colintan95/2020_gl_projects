#ifndef GFX_UTILS_TEXTURE_H_
#define GFX_UTILS_TEXTURE_H_

#include <vector>
#include <string>

namespace gfx_utils {

// Currently only supports RGB and RGBA
struct Texture {
  uint32_t tex_width;
  uint32_t tex_height;

  bool has_alpha;

  std::vector<unsigned char> tex_data; 
};

bool CreateTextureFromFile(Texture *out_tex, const std::string& path);

}

#endif