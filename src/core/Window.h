#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace mo3d {

struct WindowConfig {
    int width = 1920;
    int height = 1080;
    std::string title = "Mo3D - MIDI 3D Visualization";
    bool fullscreen = false;
    bool vsync = true;
    int samples = 4;  // MSAA samples
};

class Window {
public:
    Window();
    ~Window();

    bool Initialize(const WindowConfig& config);
    void Shutdown();

    bool ShouldClose() const;
    void PollEvents();
    void SwapBuffers();

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    float GetAspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }

    GLFWwindow* GetNativeWindow() const { return window; }

    void SetTitle(const std::string& title);
    void SetVSync(bool enabled);
    void SetFullscreen(bool enabled);
    void ToggleFullscreen();
    bool IsFullscreen() const { return isFullscreen; }

    using ResizeCallback = std::function<void(int, int)>;
    using KeyCallback = std::function<void(int, int, int, int)>;
    using MouseButtonCallback = std::function<void(int, int, int)>;
    using MouseMoveCallback = std::function<void(double, double)>;
    using ScrollCallback = std::function<void(double, double)>;

    void SetResizeCallback(ResizeCallback callback) { resizeCallback = callback; }
    void SetKeyCallback(KeyCallback callback) { keyCallback = callback; }
    void SetMouseButtonCallback(MouseButtonCallback callback) { mouseButtonCallback = callback; }
    void SetMouseMoveCallback(MouseMoveCallback callback) { mouseMoveCallback = callback; }
    void SetScrollCallback(ScrollCallback callback) { scrollCallback = callback; }

private:
    GLFWwindow* window;
    int width;
    int height;
    std::string title;
    bool isFullscreen;

    // Store windowed mode position/size for fullscreen toggle
    int windowedX, windowedY;
    int windowedWidth, windowedHeight;

    ResizeCallback resizeCallback;
    KeyCallback keyCallback;
    MouseButtonCallback mouseButtonCallback;
    MouseMoveCallback mouseMoveCallback;
    ScrollCallback scrollCallback;

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void KeyCallbackGLFW(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallbackGLFW(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallbackGLFW(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallbackGLFW(GLFWwindow* window, double xoffset, double yoffset);
};

} // namespace mo3d
