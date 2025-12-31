#include "Shader.h"
#include "GLHeaders.h"
#include "../utils/Logger.h"
#include "../utils/FileIO.h"
#include <glm/gtc/type_ptr.hpp>

namespace mo3d {

Shader::Shader() : programID(0) {}

Shader::~Shader() {
    if (programID != 0) {
        glDeleteProgram(programID);
    }
}

bool Shader::LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = FileIO::ReadTextFile(vertexPath);
    std::string fragmentSource = FileIO::ReadTextFile(fragmentPath);

    if (vertexSource.empty()) {
        LOG_ERROR("Failed to load vertex shader: ", vertexPath);
        return false;
    }

    if (fragmentSource.empty()) {
        LOG_ERROR("Failed to load fragment shader: ", fragmentPath);
        return false;
    }

    return LoadFromSource(vertexSource, fragmentSource);
}

bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    if (!CompileShader(vertexShader, vertexSource, "VERTEX")) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    if (!CompileShader(fragmentShader, fragmentSource, "FRAGMENT")) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);

    if (!LinkProgram(programID)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(programID);
        programID = 0;
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    LOG_INFO("Shader compiled successfully");
    return true;
}

bool Shader::CompileShader(unsigned int shader, const std::string& source, const std::string& type) {
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        LOG_ERROR(type, " shader compilation failed: ", infoLog);
        return false;
    }

    return true;
}

bool Shader::LinkProgram(unsigned int program) {
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(program, 1024, nullptr, infoLog);
        LOG_ERROR("Shader program linking failed: ", infoLog);
        return false;
    }

    return true;
}

void Shader::Use() const {
    if (programID != 0) {
        glUseProgram(programID);
    }
}

void Shader::Unuse() const {
    glUseProgram(0);
}

int Shader::GetUniformLocation(const std::string& name) const {
    if (uniformLocationCache.find(name) != uniformLocationCache.end()) {
        return uniformLocationCache[name];
    }

    int location = glGetUniformLocation(programID, name.c_str());
    uniformLocationCache[name] = location;
    return location;
}

void Shader::SetInt(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetFloat(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(GetUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetMat3(const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

} // namespace mo3d
