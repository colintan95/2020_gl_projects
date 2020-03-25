#version 330 core
out vec4 out_color;

in vec2 frag_texcoord;

uniform sampler2D pos_tex;
uniform sampler2D normal_tex;
uniform sampler2D ssao_tex;
uniform sampler2D ambient_tex;

struct PointLight {
  bool is_active;

  vec3 position; 
  vec3 diffuse_intensity;
  vec3 specular_intensity;

  samplerCube shadow_tex;
  float far_plane;
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
  float ssao = texture(ssao_tex, frag_texcoord).r;

  vec3 mtl_ambient = texture(ambient_tex, frag_texcoord).rgb;
  vec3 ambient = ssao * mtl_ambient * ambient_intensity;

  out_color = vec4(ambient, 1.0);
}