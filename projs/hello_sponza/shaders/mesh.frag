#version 330 core

in vec3 frag_normal;
in vec2 frag_texcoord;
flat in uint frag_mtl_id;

out vec4 out_color;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 emission;
  float shininess;
  sampler2D texture;
};

uniform Material materials[10];

void main() {
  out_color = texture(materials[frag_mtl_id].texture, frag_texcoord);
}