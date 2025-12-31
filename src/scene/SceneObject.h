#pragma once

#include "Transform.h"
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace mo3d {

class Material;
class Mesh;

enum class ObjectType {
    Mesh3D,
    Sprite2D,
    Text,
    ParticleSystem,
    Custom
};

struct RenderProperties {
    glm::vec4 color = glm::vec4(1.0f);
    float opacity = 1.0f;
    bool visible = true;
    bool castShadows = false;
    bool receiveShadows = false;
    int renderLayer = 0;
    std::string shaderName;
};

class SceneObject {
public:
    SceneObject(const std::string& name = "Object")
        : name(name)
        , objectType(ObjectType::Mesh3D)
        , enabled(true)
        , mesh(nullptr)
        , material(nullptr)
    {}

    virtual ~SceneObject() = default;

    Transform transform;
    RenderProperties renderProps;

    std::string GetName() const { return name; }
    void SetName(const std::string& n) { name = n; }

    ObjectType GetType() const { return objectType; }
    void SetType(ObjectType type) { objectType = type; }

    bool IsEnabled() const { return enabled; }
    void SetEnabled(bool e) { enabled = e; }

    void SetMesh(std::shared_ptr<Mesh> m) { mesh = m; }
    std::shared_ptr<Mesh> GetMesh() const { return mesh; }

    void SetMaterial(std::shared_ptr<Material> m) { material = m; }
    std::shared_ptr<Material> GetMaterial() const { return material; }

    virtual void Update(float deltaTime) {
        if (updateCallback) {
            updateCallback(deltaTime);
        }
    }

    void SetUpdateCallback(std::function<void(float)> callback) {
        updateCallback = callback;
    }

    // These methods removed - access transform directly instead

    glm::vec4& GetColor() { return renderProps.color; }
    float& GetOpacity() { return renderProps.opacity; }

    void FadeIn(float duration);
    void FadeOut(float duration);
    void PulseOpacity(float frequency, float min, float max);
    void RotateOverTime(const glm::vec3& axis, float speed);

private:
    std::string name;
    ObjectType objectType;
    bool enabled;

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;

    std::function<void(float)> updateCallback;

    float fadeTime = 0.0f;
    float fadeDuration = 0.0f;
    bool isFading = false;
    bool isFadingOut = false;
};

} // namespace mo3d
