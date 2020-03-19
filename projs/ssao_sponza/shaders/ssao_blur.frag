#version 330 core
out float out_occlusion;

in vec2 frag_texcoord;

uniform sampler2D ssao_tex;

void main() {
  vec2 texel_size = 1.0 / vec2(textureSize(ssao_tex, 0));

  float result = 0.0;

  for (int x = -2; x < 2; ++x) {
    for (int y = -2; y < 2; ++y) {
      vec2 offset = vec2(float(x), float(y)) * texel_size;
      result += texture(ssao_tex, frag_texcoord + offset).r;
    }
  }

  out_occlusion = result / (4.0 * 4.0);
}