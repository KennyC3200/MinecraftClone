#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

namespace mcc {

class Shader final {
public:
    explicit Shader(const std::string& vert_path, const std::string& frag_path);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&&) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    [[nodiscard]] GLuint Handle() const & { return m_handle; }
    GLuint Handle() const && = delete;

    void Bind() const;

    void SetUniform(const std::string& name, int val) const;
    void SetUniform(const std::string& name, float val) const;
    void SetUniform(const std::string& name, const glm::vec3& val) const;
    void SetUniform(const std::string& name, const glm::mat4& val) const;

private:
    GLint UniformLoc(const std::string& name) const;

    GLuint m_handle = 0;
    std::string m_vert_path, m_frag_path;
};

}