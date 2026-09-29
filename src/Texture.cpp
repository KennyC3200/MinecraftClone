#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <utility>
#include <stdexcept>

namespace mcc {

Texture::Texture(const std::string& path, GLint src_fmt, GLint tex_fmt) {
    glGenTextures(1, &m_handle);
    glBindTexture(GL_TEXTURE_2D, m_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &m_size.x, &m_size.y, &channels, 0);
    if (data) {
        glTexImage2D(
            GL_TEXTURE_2D, 0, tex_fmt, 
            m_size.x, m_size.y, 0, src_fmt, 
            GL_UNSIGNED_BYTE, data);
    } else {
        glDeleteTextures(1, &m_handle);
        throw std::runtime_error("Failed to load texture: " + path);
    }
    stbi_image_free(data);
}

Texture::~Texture() {
    glDeleteTextures(1, &m_handle);
}

Texture::Texture(Texture&& other) noexcept 
    : m_handle(std::exchange(other.m_handle, 0)) 
    , m_size(std::exchange(other.m_size, {0, 0})) {}

Texture& Texture::operator=(Texture&& other) noexcept {
    std::swap(m_handle, other.m_handle);
    std::swap(m_size, other.m_size);
    return *this;
}

void Texture::Bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_handle);
}

}
