#version 330 core

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_texcoord;
flat in uint frag_mtl_id;

out vec4 out_color;

uniform vec3 camera_pos;

struct PointLight {
  vec3 position;
  vec3 diffuse_intensity;
  vec3 specular_intensity;
};

struct Material {
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 emission_color;
  float shininess;

  bool has_ambient_tex;
  bool has_diffuse_tex;
  bool has_specular_tex;

  sampler2D ambient_texture;
  sampler2D diffuse_texture;
  sampler2D specular_texture;
};

uniform PointLight lights[5];
uniform Material materials[5];

uniform vec3 ambient_intensity;

void main() {   
  vec3 mtl_emission = materials[frag_mtl_id].emission_color;
  vec3 mtl_ambient  = materials[frag_mtl_id].ambient_color;
  vec3 mtl_diffuse  = materials[frag_mtl_id].diffuse_color;
  vec3 mtl_specular = materials[frag_mtl_id].specular_color;

  if (materials[frag_mtl_id].has_ambient_tex) {
    mtl_ambient *= texture(materials[frag_mtl_id].ambient_texture, 
                           frag_texcoord).rgb;
  }
  if (materials[frag_mtl_id].has_diffuse_tex) {
    mtl_diffuse *= texture(materials[frag_mtl_id].diffuse_texture,
                           frag_texcoord).rgb;
  }
  if (materials[frag_mtl_id].has_specular_tex) {
    mtl_specular *= texture(materials[frag_mtl_id].specular_texture,
                            frag_texcoord).rgb;
  }

  vec3 emission = mtl_emission;
  vec3 ambient  = mtl_ambient * ambient_intensity;
  vec3 diffuse  = vec3(0.0);
  vec3 specular = vec3(0.0);

  // TODO(colintan): Should we have a uniform variable to tell us the number
  // of light sources?
  for (int i = 0; i < 5; ++i) {
    vec3 light_vec = lights[i].position - frag_pos;
    vec3 view_vec = camera_pos - frag_pos;

    float light_dist = length(light_vec);
    float attenuation = pow(20.0 / max(light_dist, 20.0), 2);

    // TODO(colintan): Remove this once everything else is working
    attenuation = 1;

    float diffuse_dot = max(dot(normalize(light_vec), normalize(frag_normal)), 
                            0.0);
    float diffuse_coeff = attenuation * diffuse_dot;

    vec3 half_vec = normalize(normalize(light_vec) + normalize(view_vec));

    float specular_dot = max(dot(half_vec, normalize(frag_normal)), 0.0);
    float specular_coeff = attenuation * 
        pow(specular_dot, materials[frag_mtl_id].shininess);

    diffuse  += mtl_diffuse  * lights[i].diffuse_intensity  * diffuse_coeff;
    specular += mtl_specular * lights[i].specular_intensity * specular_coeff;
  }

  out_color = vec4(emission + ambient + diffuse + specular, 1.0);
}