#pragma once

#include <chrono>

namespace mo3d {

class Time {
public:
    static void Init() {
        startTime = std::chrono::high_resolution_clock::now();
        lastFrameTime = startTime;
    }

    static void Update() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        totalTime = std::chrono::duration<float>(currentTime - startTime).count();
        lastFrameTime = currentTime;
        frameCount++;
    }

    static float DeltaTime() { return deltaTime; }
    static float TotalTime() { return totalTime; }
    static uint64_t FrameCount() { return frameCount; }
    static float FPS() { return deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f; }

private:
    inline static std::chrono::high_resolution_clock::time_point startTime;
    inline static std::chrono::high_resolution_clock::time_point lastFrameTime;
    inline static float deltaTime = 0.0f;
    inline static float totalTime = 0.0f;
    inline static uint64_t frameCount = 0;
};

} // namespace mo3d
