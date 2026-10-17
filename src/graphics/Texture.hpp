#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

namespace mcc {

class Texture final {
public:
    explicit Texture(const std::string& path, GLint src_fmt, GLint tex_fmt);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void Bind(GLuint unit) const;

    GLuint Handle() const { return m_handle; }
    const glm::ivec2& Size() const { return m_size; }

private:
    GLuint m_handle = 0;
    glm::ivec2 m_size{0, 0};
};

}
