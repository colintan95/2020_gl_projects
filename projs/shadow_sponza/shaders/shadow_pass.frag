#version 330 core

in vec4 frag_pos;

uniform vec3 light_pos;
uniform float far_plane;

void main() {
  float light_dist = length(frag_pos.xyz - light_pos) / far_plane;

  gl_FragDepth = light_dist;
}