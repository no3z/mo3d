#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace mo3d {

class Shader {
public:
    Shader();
    ~Shader();

    bool LoadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);

    void Use() const;
    void Unuse() const;

    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, const glm::vec2& value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetVec4(const std::string& name, const glm::vec4& value);
    void SetMat3(const std::string& name, const glm::mat3& value);
    void SetMat4(const std::string& name, const glm::mat4& value);

    unsigned int GetProgramID() const { return programID; }
    bool IsValid() const { return programID != 0; }

private:
    unsigned int programID;
    mutable std::unordered_map<std::string, int> uniformLocationCache;

    int GetUniformLocation(const std::string& name) const;
    bool CompileShader(unsigned int shader, const std::string& source, const std::string& type);
    bool LinkProgram(unsigned int program);
};

} // namespace mo3d
