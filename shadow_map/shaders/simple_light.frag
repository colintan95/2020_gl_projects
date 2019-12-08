#version 330 core

in vec3 frag_pos;
in vec3 frag_normal;

out vec4 out_color;

uniform vec3 mesh_color;

uniform vec3 light_loc;
uniform vec3 camera_loc;

void main() {
  vec3 light_vec = light_loc - frag_pos;
  vec3 view_vec = normalize(camera_loc - frag_pos);

  float light_dist = length(light_vec);
  float diffuse_atten = pow(20.0 / max(light_dist, 20.0), 2);

  float dot_prod = max(dot(normalize(light_vec), normalize(frag_normal)), 0.0);

  out_color = vec4(diffuse_atten * dot_prod * vec3(0.3, 0.3, 0.3) + mesh_color, 
                   1.0);
}