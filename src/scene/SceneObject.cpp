#include "SceneObject.h"
#include "../core/Time.h"
#include <glm/gtc/constants.hpp>

namespace mo3d {

void SceneObject::FadeIn(float duration) {
    fadeDuration = duration;
    fadeTime = 0.0f;
    isFading = true;
    isFadingOut = false;

    SetUpdateCallback([this](float deltaTime) {
        if (!isFading) return;

        fadeTime += deltaTime;
        float t = fadeTime / fadeDuration;

        if (t >= 1.0f) {
            renderProps.opacity = 1.0f;
            isFading = false;
        } else {
            renderProps.opacity = t;
        }
    });
}

void SceneObject::FadeOut(float duration) {
    fadeDuration = duration;
    fadeTime = 0.0f;
    isFading = true;
    isFadingOut = true;

    SetUpdateCallback([this](float deltaTime) {
        if (!isFading) return;

        fadeTime += deltaTime;
        float t = fadeTime / fadeDuration;

        if (t >= 1.0f) {
            renderProps.opacity = 0.0f;
            isFading = false;
            enabled = false;
        } else {
            renderProps.opacity = 1.0f - t;
        }
    });
}

void SceneObject::PulseOpacity(float frequency, float min, float max) {
    SetUpdateCallback([this, frequency, min, max](float deltaTime) {
        float t = glm::sin(Time::TotalTime() * frequency * glm::two_pi<float>());
        renderProps.opacity = min + (t * 0.5f + 0.5f) * (max - min);
    });
}

void SceneObject::RotateOverTime(const glm::vec3& axis, float speed) {
    SetUpdateCallback([this, axis, speed](float deltaTime) {
        glm::vec3 rotationDelta = axis * speed * deltaTime;
        transform.RotateEuler(rotationDelta);
    });
}

} // namespace mo3d
