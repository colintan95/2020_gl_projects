#version 330 core

in vec2 frag_texcoord;

out vec4 out_color;

uniform sampler2D mesh_texture;

void main() {
  out_color = texture(mesh_texture, frag_texcoord);
}