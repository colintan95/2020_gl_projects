#version 330 core
layout(location = 0) out vec3 out_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_albedo_spec;

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_texcoords;

uniform sampler2D diffuse_tex;
uniform sampler2D specular_tex;

void main() {
  out_pos    = frag_pos;
  out_normal = normalize(frag_normal);

  out_albedo_spec.rgb = texture(diffuse_tex, frag_texcoords).rgb;
  out_albedo_spec.a   = texture(specular_tex, frag_texcoords).r;
}