#include "Renderer.h"
#include "GLHeaders.h"
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"
#include "../scene/Scene.h"
#include "../scene/Camera.h"
#include "../scene/SceneObject.h"
#include "../utils/Logger.h"

namespace mo3d {

Renderer::Renderer()
    : clearColor(0.1f, 0.1f, 0.1f)
    , wireframeMode(false)
{
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    InitializeDefaultShader();

    LOG_INFO("Renderer initialized");
    return true;
}

void Renderer::Shutdown() {
    defaultShader.reset();
}

void Renderer::BeginFrame() {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void Renderer::EndFrame() {
}

void Renderer::RenderScene(Scene* scene, Camera* camera) {
    if (!scene || !camera) return;

    glm::mat4 projection = camera->GetProjectionMatrix();
    glm::mat4 view = camera->GetViewMatrix();

    auto objects = scene->GetAllObjects();

    for (auto& object : objects) {
        if (!object || !object->IsEnabled() || !object->renderProps.visible) {
            continue;
        }

        auto mesh = object->GetMesh();
        if (!mesh) continue;

        auto material = object->GetMaterial();
        auto shader = (material && material->shader) ? material->shader : defaultShader;

        if (!shader || !shader->IsValid()) continue;

        shader->Use();

        glm::mat4 model = object->transform.GetMatrix();
        shader->SetMat4("uModel", model);
        shader->SetMat4("uView", view);
        shader->SetMat4("uProjection", projection);

        glm::vec4 finalColor = object->renderProps.color;
        if (material) {
            finalColor *= material->albedo;
        }
        finalColor.a *= object->renderProps.opacity;

        shader->SetVec4("uColor", finalColor);
        shader->SetFloat("uTime", 0.0f);

        mesh->Draw();
    }
}

void Renderer::SetClearColor(const glm::vec3& color) {
    clearColor = color;
}

void Renderer::SetWireframe(bool enabled) {
    wireframeMode = enabled;
}

void Renderer::InitializeDefaultShader() {
    std::string vertexShader = R"(
        #version 410 core
        layout (location = 0) in vec3 aPosition;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoord;
        layout (location = 3) in vec4 aColor;

        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;

        out vec3 vNormal;
        out vec2 vTexCoord;
        out vec4 vColor;

        void main() {
            gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
            vNormal = mat3(transpose(inverse(uModel))) * aNormal;
            vTexCoord = aTexCoord;
            vColor = aColor;
        }
    )";

    std::string fragmentShader = R"(
        #version 410 core
        in vec3 vNormal;
        in vec2 vTexCoord;
        in vec4 vColor;

        uniform vec4 uColor;
        uniform float uTime;

        out vec4 FragColor;

        void main() {
            vec3 normal = normalize(vNormal);
            vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 lighting = vec3(0.3) + vec3(0.7) * diff;

            FragColor = uColor * vec4(lighting, 1.0);
        }
    )";

    defaultShader = std::make_shared<Shader>();
    if (!defaultShader->LoadFromSource(vertexShader, fragmentShader)) {
        LOG_ERROR("Failed to create default shader");
        defaultShader.reset();
    }
}

} // namespace mo3d
