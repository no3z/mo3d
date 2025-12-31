#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace mo3d {

class Transform {
public:
    Transform()
        : position(0.0f)
        , rotation(1.0f, 0.0f, 0.0f, 0.0f)  // Identity quaternion
        , scale(1.0f)
        , dirty(true)
    {}

    // Position
    void SetPosition(const glm::vec3& pos) { position = pos; dirty = true; }
    void SetPosition(float x, float y, float z) { position = glm::vec3(x, y, z); dirty = true; }
    const glm::vec3& GetPosition() const { return position; }

    // Rotation (using quaternions for smooth interpolation)
    void SetRotation(const glm::quat& rot) { rotation = rot; dirty = true; }
    void SetRotationEuler(const glm::vec3& euler) {
        rotation = glm::quat(euler);
        dirty = true;
    }
    void SetRotationEuler(float x, float y, float z) {
        SetRotationEuler(glm::vec3(x, y, z));
    }
    const glm::quat& GetRotation() const { return rotation; }
    glm::vec3 GetRotationEuler() const { return glm::eulerAngles(rotation); }

    // Scale
    void SetScale(const glm::vec3& scl) { scale = scl; dirty = true; }
    void SetScale(float s) { scale = glm::vec3(s); dirty = true; }
    void SetScale(float x, float y, float z) { scale = glm::vec3(x, y, z); dirty = true; }
    const glm::vec3& GetScale() const { return scale; }

    // Matrix
    const glm::mat4& GetMatrix() const {
        if (dirty) {
            UpdateMatrix();
        }
        return matrix;
    }

    // Transform operations
    void Translate(const glm::vec3& offset) {
        position += offset;
        dirty = true;
    }

    void Rotate(const glm::quat& rot) {
        rotation = rot * rotation;
        dirty = true;
    }

    void RotateEuler(const glm::vec3& euler) {
        Rotate(glm::quat(euler));
    }

    void ScaleBy(const glm::vec3& factor) {
        scale *= factor;
        dirty = true;
    }

    // Direction vectors
    glm::vec3 Forward() const {
        return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::vec3 Right() const {
        return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 Up() const {
        return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

private:
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    mutable glm::mat4 matrix;
    mutable bool dirty;

    void UpdateMatrix() const {
        matrix = glm::translate(glm::mat4(1.0f), position) *
                 glm::mat4_cast(rotation) *
                 glm::scale(glm::mat4(1.0f), scale);
        dirty = false;
    }
};

} // namespace mo3d
