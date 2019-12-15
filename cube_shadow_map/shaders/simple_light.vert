#version 330 core

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec3 vert_normal;
layout(location = 3) in uint vert_mtl_id;

out vec3 frag_pos;
out vec3 frag_normal;
flat out uint frag_mtl_id;

out vec4 frag_shadow_coords[5]; // one for each light

uniform mat4 mv_mat;
uniform mat4 mvp_mat;
uniform mat3 normal_mat;

uniform mat4 shadow_mats[5]; // proj * view matrices from the light's viewpoint

void main() {
  gl_Position = mvp_mat * vec4(vert_pos, 1.0);
  
  frag_pos = (mv_mat * vec4(vert_pos, 1.0)).xyz;
  frag_normal = normal_mat * vert_normal;
  frag_mtl_id = vert_mtl_id;

  for (int i = 0; i < 5; ++i) {
    frag_shadow_coords[i] = shadow_mats[i] * vec4(vert_pos, 1.0);
  }
}