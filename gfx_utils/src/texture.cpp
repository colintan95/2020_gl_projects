#include "gfx_utils/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>

namespace gfx_utils {

bool CreateTextureFromFile(Texture* out_tex, const std::string& tex_directory,
                           const std::string& texname) {
  out_tex->tex_width = 0;
  out_tex->tex_height = 0;
  out_tex->has_alpha = false;
  out_tex->tex_data.clear();

  int tex_width = -1;
  int tex_height = -1;
  int channels = -1;

  std::string path = tex_directory + "/" + texname;
  stbi_set_flip_vertically_on_load(true);
  stbi_uc *pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &channels, 
                              0);

  if (!pixels) {
    return false;
  }

  out_tex->tex_width = static_cast<uint32_t>(tex_width);
  out_tex->tex_height = static_cast<uint32_t>(tex_height);
  
  out_tex->has_alpha = false;
  if (channels == 4) {
    out_tex->has_alpha = true;
  }

  uint32_t tex_size = out_tex->tex_width * out_tex->tex_height * channels;
  out_tex->tex_data = std::vector<unsigned char>(pixels, pixels + tex_size);

  return true;
}

}