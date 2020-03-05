#version 330 core
out vec4 out_color;
// out float out_occlusion;

in vec2 frag_texcoord;

uniform sampler2D pos_tex;
uniform sampler2D normal_tex;
uniform sampler2D noise_tex;

uniform vec3 samples[64];

uniform mat4 proj_mat;

int kernel_size = 64;
float radius = 0.5;
float bias = 0.025;

const vec2 noise_scale = vec2(1920.0 / 4.0, 1080.0 / 4.0);

void main() {
  vec3 pos = texture(pos_tex, frag_texcoord).xyz;
  vec3 normal = normalize(texture(normal_tex, frag_texcoord).rgb);

  vec3 rand_vec = 
      normalize(texture(noise_tex, frag_texcoord * noise_scale).xyz);

  vec3 tangent = normalize(rand_vec - normal * dot(rand_vec, normal));
  vec3 bitangent = cross(normal, tangent);

  // Converts from tangent space to view space
  mat3 tbn = mat3(tangent, bitangent, normal);

  float occlusion = 0.0;
  
  for (int i = 0; i < kernel_size; ++i) {
    vec3 sample = tbn * samples[i];
    sample = pos + sample * radius;

    vec4 offset = vec4(sample, 1.0);
    offset = proj_mat * offset;
    offset.xyz /= offset.w;
    offset.xyz = offset.xyz * 0.5 + 0.5;

    float sample_depth = texture(pos_tex, offset.xy).z;

    float range_check = 
        smoothstep(0.0, 1.0, radius / abs(pos.z - sample_depth));
    occlusion += (sample_depth >= sample.z + bias ? 1.0 : 0.0) * range_check;
  }

  occlusion = 1.0 - (occlusion / kernel_size);

  // out_occlusion = occlusion

  out_color = vec4(occlusion, occlusion, occlusion, 1);
}