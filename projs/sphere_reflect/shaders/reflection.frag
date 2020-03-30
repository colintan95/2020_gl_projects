#version 330 core

out vec4 out_color;

in vec3 frag_pos;
in vec3 frag_normal;

uniform vec3 camera_pos;
uniform samplerCube cubemap;

void main() {
  vec3 incident_ray = normalize(frag_pos - camera_pos);
  vec3 reflect_ray = reflect(incident_ray, normalize(frag_normal));

  out_color = vec4(texture(cubemap, reflect_ray).rgb, 1.0);
  // out_color = vec4(1.0, 0.0, 0.0, 1.0);
}