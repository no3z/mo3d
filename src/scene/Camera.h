#pragma once

#include "Transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mo3d {

enum class ProjectionMode {
    Perspective,
    Orthographic
};

class Camera {
public:
    Camera()
        : projectionMode(ProjectionMode::Perspective)
        , fov(45.0f)
        , aspectRatio(16.0f / 9.0f)
        , nearPlane(0.1f)
        , farPlane(1000.0f)
        , orthoSize(10.0f)
        , projectionDirty(true)
    {}

    Transform transform;

    void SetPerspective(float fovDegrees, float aspect, float near, float far) {
        projectionMode = ProjectionMode::Perspective;
        fov = fovDegrees;
        aspectRatio = aspect;
        nearPlane = near;
        farPlane = far;
        projectionDirty = true;
    }

    void SetOrthographic(float size, float aspect, float near, float far) {
        projectionMode = ProjectionMode::Orthographic;
        orthoSize = size;
        aspectRatio = aspect;
        nearPlane = near;
        farPlane = far;
        projectionDirty = true;
    }

    void SetFOV(float fovDegrees) {
        fov = fovDegrees;
        projectionDirty = true;
    }

    void SetAspectRatio(float aspect) {
        aspectRatio = aspect;
        projectionDirty = true;
    }

    float GetFOV() const { return fov; }
    float GetAspectRatio() const { return aspectRatio; }
    float GetNearPlane() const { return nearPlane; }
    float GetFarPlane() const { return farPlane; }

    const glm::mat4& GetProjectionMatrix() const {
        if (projectionDirty) {
            UpdateProjectionMatrix();
        }
        return projectionMatrix;
    }

    glm::mat4 GetViewMatrix() const {
        glm::vec3 pos = transform.GetPosition();
        glm::vec3 forward = transform.Forward();
        glm::vec3 up = transform.Up();
        return glm::lookAt(pos, pos + forward, up);
    }

    glm::mat4 GetViewProjectionMatrix() const {
        return GetProjectionMatrix() * GetViewMatrix();
    }

    void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f)) {
        glm::vec3 pos = transform.GetPosition();
        glm::vec3 direction = glm::normalize(target - pos);

        float pitch = glm::asin(-direction.y);
        float yaw = glm::atan(direction.x, -direction.z);

        transform.SetRotationEuler(pitch, yaw, 0.0f);
    }

    void Orbit(const glm::vec3& center, float deltaYaw, float deltaPitch, float distance) {
        glm::vec3 euler = transform.GetRotationEuler();
        euler.y += deltaYaw;
        euler.x += deltaPitch;

        euler.x = glm::clamp(euler.x, -glm::half_pi<float>() + 0.1f, glm::half_pi<float>() - 0.1f);

        transform.SetRotationEuler(euler);

        glm::vec3 direction = -transform.Forward();
        transform.SetPosition(center + direction * distance);
    }

private:
    ProjectionMode projectionMode;
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    float orthoSize;

    mutable glm::mat4 projectionMatrix;
    mutable bool projectionDirty;

    void UpdateProjectionMatrix() const {
        if (projectionMode == ProjectionMode::Perspective) {
            projectionMatrix = glm::perspective(
                glm::radians(fov),
                aspectRatio,
                nearPlane,
                farPlane
            );
        } else {
            float halfWidth = orthoSize * aspectRatio * 0.5f;
            float halfHeight = orthoSize * 0.5f;
            projectionMatrix = glm::ortho(
                -halfWidth, halfWidth,
                -halfHeight, halfHeight,
                nearPlane, farPlane
            );
        }
        projectionDirty = false;
    }
};

} // namespace mo3d
