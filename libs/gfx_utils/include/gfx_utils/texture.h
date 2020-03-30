#ifndef GFX_UTILS_TEXTURE_H_
#define GFX_UTILS_TEXTURE_H_

#include <vector>
#include <string>
#include <cstdint>

namespace gfx_utils {

enum ImageFormat {
  kImageFormatInvalid,
  kImageFormatRGB,
  kImageFormatRGBA
};

struct Image {
  uint32_t width = 0;
  uint32_t height = 0;

  ImageFormat format = kImageFormatInvalid;

  std::vector<uint8_t> data;
};

using TextureId = uint64_t;

// Currently only supports RGB and RGBA
struct Texture {
  TextureId id; // Assigned on construction

  Image image;

  // Constructor to assign the id
  Texture();
};

using CubemapId = uint64_t;

struct Cubemap {
  CubemapId id;

  // The order of faces (e.g. right, left, ...) should NOT be changed
  // It allows us to merely add the index to GL_TEXTURE_CUB_MAP_POSITIVE_X
  // when we call glTexImage2D() for each face
  std::vector<Image> images;

  // Constructor to assign the id
  Cubemap();
};

// flip should be true for 2D textures, and false for cubemaps
bool LoadImageFromFile(Image* out_img, const std::string& path, bool flip);

bool CreateTextureFromFile(Texture* out_tex, const std::string& tex_directory,
                           const std::string& texname);

bool CreateCubemapFromFiles(Cubemap* out_cubemap, const std::string& directory);

}

#endif