#ifndef GFX_UTILS_PROGRAM_H_
#define GFX_UTILS_PROGRAM_H_

#include <GL/glew.h>
#if defined(WIN32)
#include <GL/gl.h>
#endif

#include <string>

#include <glm/matrix.hpp>

namespace gfx_utils {

// Don't create directly. Use only when a Program object returns a Uniform
// class
class Uniform {
public:
  void Set(bool val);
  void Set(int val);
  void Set(float val);
  void Set(const glm::vec3& val);
  void Set(const glm::mat3& val);
  void Set(const glm::mat4& val);

private:
  Uniform(GLint location) { location_ = location; }

private:
  GLint location_;

private:
  friend class Program;
};

class Program {
public:
  bool CreateProgram(const std::string& vert_shader_path, 
                     const std::string& frag_shader_path);
  void DestroyProgram();

  // e.g. program.GetUniform("mvp_mat").Set(...)
  Uniform GetUniform(const std::string& var);
  Uniform GetUniform(const std::string& array, int index, 
                     const std::string& var);

  GLuint GetProgramId() const { return program_id_; }

private:
  bool LoadShaderSource(std::string* out_str, const std::string& path);

private:
  GLuint program_id_;

  bool is_created_;
};

} // namespace gfx_utils

#endif // GFX_UTILS_PROGORAM_H_