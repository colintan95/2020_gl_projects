#version 330 core

in vec3 frag_pos;
in vec3 frag_normal;
flat in uint frag_mtl_id;

out vec4 out_color;

uniform vec3 camera_pos;

struct PointLight {
  vec3 position;
  vec3 diffuse_color;
  vec3 specular_color;
};

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 emission;
  float shininess;
};

uniform PointLight lights[5];
uniform Material materials[5];

uniform vec3 ambient_color;

void main() {   
  vec3 emission = materials[frag_mtl_id].emission;
  vec3 ambient = materials[frag_mtl_id].ambient * ambient_color;
  vec3 diffuse = vec3(0.0, 0.0, 0.0);
  vec3 specular = vec3(0.0, 0.0, 0.0);

  // TODO(colintan): Should we have a uniform variable to tell us the number
  // of light sources?
  for (int i = 0; i < 5; ++i) {
    vec3 light_vec = lights[i].position - frag_pos;
    vec3 view_vec = camera_pos - frag_pos;

    float light_dist = length(light_vec);
    float attenuation = pow(20.0 / max(light_dist, 20.0), 2);
    attenuation = 1;

    float diffuse_dot = 
        max(dot(normalize(light_vec), normalize(frag_normal)), 0.0);
    float diffuse_coeff = attenuation * diffuse_dot;

    vec3 half_vec = normalize(normalize(light_vec) + normalize(view_vec));
    float specular_dot = max(dot(half_vec, normalize(frag_normal)), 0.0);
    float specular_coeff = attenuation * 
        pow(specular_dot, materials[frag_mtl_id].shininess);

    diffuse += materials[frag_mtl_id].diffuse * lights[i].diffuse_color * 
        diffuse_coeff;
    specular += materials[frag_mtl_id].specular * lights[i].specular_color * 
        specular_coeff;
  }

  out_color = vec4(emission + ambient + diffuse + specular, 1.0);
}