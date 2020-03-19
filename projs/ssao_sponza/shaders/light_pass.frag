#version 330 core
out vec4 out_color;

in vec2 frag_texcoord;

uniform sampler2D pos_tex;
uniform sampler2D normal_tex;
uniform sampler2D ssao_tex;
uniform sampler2D ambient_tex;

uniform vec3 ambient_intensity;

void main() {
  float ssao = texture(ssao_tex, frag_texcoord).r;

  vec3 mtl_ambient = texture(ambient_tex, frag_texcoord).rgb;
  vec3 ambient = ssao * mtl_ambient * ambient_intensity;

  out_color = vec4(ambient, 1.0);
}