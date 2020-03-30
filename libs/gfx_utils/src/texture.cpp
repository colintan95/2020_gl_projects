#include "gfx_utils/texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>

namespace gfx_utils {

static TextureId texture_id_counter = 0;

static CubemapId cubemap_id_counter = 0;

Texture::Texture() {
  ++texture_id_counter;

  id = texture_id_counter;
}

Cubemap::Cubemap() {
  ++cubemap_id_counter;

  id = cubemap_id_counter;
}

bool LoadImageFromFile(Image* out_img, const std::string& path, bool flip) {
  out_img->width = 0;
  out_img->height = 0;
  out_img->format = kImageFormatInvalid;
  out_img->data.clear();

  int load_width = -1;
  int load_height = -1;
  int load_channels = -1;

  if (flip) {
    stbi_set_flip_vertically_on_load(true);
  }

  stbi_uc *pixels = stbi_load(path.c_str(), &load_width, &load_height, 
                              &load_channels, 0);
  
  if (!pixels) {
    return false;
  }

  out_img->width = static_cast<uint32_t>(load_width);
  out_img->height  = static_cast<uint32_t>(load_height);
  
  if (load_channels == 4) {
    out_img->format = kImageFormatRGBA;
  }
  else if (load_channels == 3) {
    out_img->format = kImageFormatRGB;
  }
  else {
    out_img->format = kImageFormatInvalid;
  }

  uint32_t load_size = load_width * load_height * load_channels;
  out_img->data = std::vector<unsigned char>(pixels, pixels + load_size);

  return true;
}

bool CreateTextureFromFile(Texture* out_tex, const std::string& tex_directory,
                           const std::string& texname) {
  std::string path = tex_directory + "/" + texname;

  return LoadImageFromFile(&out_tex->image, path, true);
}

bool CreateCubemapFromFiles(Cubemap* out_cubemap, 
                            const std::string& directory) {
  out_cubemap->images.resize(6);                            

  static const std::vector<std::string> filenames = {
    "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"
  };

  for (int i = 0; i < filenames.size(); ++i) {
    Image* img = &(out_cubemap->images[i]);

    const std::string& filename = filenames[i];
    std::string path = directory + "/" + filename;

    if (!LoadImageFromFile(img, path, false)) {
      return false;
    }
  }

  return true;
}

}