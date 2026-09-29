#include "graphics/Shader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <stdexcept>

namespace {

GLenum Compile(const std::string& path, GLuint type) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open shader: " + path);
    }

    std::string text;
    std::stringstream stream;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        stream << file.rdbuf();
        file.close();
        text = stream.str();
    } catch (const std::ifstream::failure& e) {
        std::cout << "Error reading from shader file: " + path;
        throw;
    }

    GLuint handle = glCreateShader(type);

    const char* text_src = text.c_str();
    int text_len = text.length();
    glShaderSource(handle, 1, &text_src, &text_len);
    glCompileShader(handle);

    int result;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int len;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &len);
        std::string msg(len, '\0');
        glGetShaderInfoLog(handle, len, &len, msg.data());

        glDeleteShader(handle);
        throw std::runtime_error("Failed to compile " + path + ":\n" + msg);
    }

    return handle;
}

}

namespace mcc {

Shader::Shader(const std::string& vert_path, const std::string& frag_path)
    : m_vert_path(vert_path)
    , m_frag_path(frag_path) 
{
    // Vert and frag shader objects
    GLuint vert = Compile(vert_path, GL_VERTEX_SHADER);
    GLuint frag = 0;
    try {
        frag = Compile(frag_path, GL_FRAGMENT_SHADER);
    } catch (...) {
        glDeleteShader(vert);
        throw;
    }

    m_handle = glCreateProgram();

    // Attach shader objects (vert and frag shader objects) to the shader program
    glAttachShader(m_handle, vert);
    glAttachShader(m_handle, frag);
    glLinkProgram(m_handle);

    // Ensure shader program successfully linked
    int result;
    glGetProgramiv(m_handle, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        int len;
        glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &len);
        std::string msg(len, '\0');
        glGetProgramInfoLog(m_handle, len, &len, msg.data());

        glDeleteProgram(m_handle);
        glDeleteShader(vert);
        glDeleteShader(frag);

        throw std::runtime_error("Failed to link program (" + 
            m_vert_path + ", " + m_frag_path + "):\n" + msg);
    }

    // Delete shader object handles once shader program is done
    glDeleteShader(vert);
    glDeleteShader(frag);
}

Shader::~Shader() {
    glDeleteProgram(m_handle);
}

Shader::Shader(Shader&& other) noexcept 
    : m_handle(std::exchange(other.m_handle, 0))
    , m_vert_path(std::move(other.m_vert_path))
    , m_frag_path(std::move(other.m_frag_path))
{}

Shader& Shader::operator=(Shader&& other) noexcept {
    std::swap(m_handle, other.m_handle);
    std::swap(m_vert_path, other.m_vert_path);
    std::swap(m_frag_path, other.m_frag_path);
    return *this;
}

void Shader::Bind() const {
    glUseProgram(m_handle);
}

void Shader::SetUniform(const std::string& name, int val) const {
    glProgramUniform1i(m_handle, UniformLoc(name), val);
}

void Shader::SetUniform(const std::string& name, float val) const {
    glProgramUniform1f(m_handle, UniformLoc(name), val);
}

void Shader::SetUniform(const std::string& name, const glm::vec3& val) const {
    glProgramUniform3fv(m_handle, UniformLoc(name), 1, glm::value_ptr(val));
}

void Shader::SetUniform(const std::string& name, const glm::mat4& val) const {
    glProgramUniformMatrix4fv(m_handle, UniformLoc(name), 1, GL_FALSE, glm::value_ptr(val));
}

GLint Shader::UniformLoc(const std::string& name) const {
    GLint loc = glGetUniformLocation(m_handle, name.c_str());
#ifndef NDEBUG
    if (loc == -1) {
        std::cerr << "WARNING: uniform " << name << " not found in shader ("
            << m_vert_path << ", " << m_frag_path << ")\n";
    }
#endif
    return loc;
}

}