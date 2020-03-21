#version 330 core
layout(location = 0) out vec3 out_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_ambient;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_texcoord;
flat in uint frag_mtl_id;

// For now, only ambient component
struct Material {
  vec3 ambient_color;
  bool has_ambient_tex;
  sampler2D ambient_texture;
};

uniform Material materials[5];

void main() {
  out_pos     = frag_pos;
  out_normal  = normalize(frag_normal);
  
  vec3 mtl_ambient  = materials[frag_mtl_id].ambient_color;
  if (materials[frag_mtl_id].has_ambient_tex) {
    mtl_ambient *= texture(materials[frag_mtl_id].ambient_texture, 
                           frag_texcoord).rgb;
  }

  out_ambient = mtl_ambient;
}