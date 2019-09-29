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
};

uniform Material materials[10];

void main() {
  out_color = vec4(frag_texcoord, 0.0, 1.0);
}