#ifndef GFX_UTILS_TEXTURE_H_
#define GFX_UTILS_TEXTURE_H_

#include <vector>
#include <string>

namespace gfx_utils {

using TextureId = uint64_t;

// Currently only supports RGB and RGBA
struct Texture {
  TextureId id; // Assigned on construction

  uint32_t tex_width = 0;
  uint32_t tex_height = 0;

  bool has_alpha = false;

  // TODO(colintan): Is this default initialized?
  std::vector<unsigned char> tex_data; 

  // Constructor to assign the id
  Texture();
};

bool CreateTextureFromFile(Texture *out_tex, const std::string& tex_directory,
                           const std::string& texname);

}

#endif