#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace mo3d {

class Scene;
class Camera;
class Shader;

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void RenderScene(Scene* scene, Camera* camera);

    void SetClearColor(const glm::vec3& color);
    void SetWireframe(bool enabled);

private:
    std::shared_ptr<Shader> defaultShader;
    glm::vec3 clearColor;
    bool wireframeMode;

    void InitializeDefaultShader();
};

} // namespace mo3d
