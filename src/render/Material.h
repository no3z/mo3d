#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace mo3d {

class Shader;
class Texture;

struct Material {
    glm::vec4 albedo = glm::vec4(1.0f);
    float metallic = 0.0f;
    float roughness = 0.5f;
    float emissive = 0.0f;

    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> albedoTexture;

    Material() = default;
};

} // namespace mo3d
