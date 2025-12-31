#include "Window.h"
#include "../utils/Logger.h"

namespace mo3d {

Window::Window()
    : window(nullptr)
    , width(0)
    , height(0)
    , isFullscreen(false)
    , windowedX(100)
    , windowedY(100)
    , windowedWidth(1920)
    , windowedHeight(1080)
{
}

Window::~Window() {
    Shutdown();
}

bool Window::Initialize(const WindowConfig& config) {
    width = config.width;
    height = config.height;
    title = config.title;
    isFullscreen = config.fullscreen;

    windowedWidth = width;
    windowedHeight = height;

    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, config.samples);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWmonitor* monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    window = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);

    if (!window) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(config.vsync ? 1 : 0);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetKeyCallback(window, KeyCallbackGLFW);
    glfwSetMouseButtonCallback(window, MouseButtonCallbackGLFW);
    glfwSetCursorPosCallback(window, CursorPosCallbackGLFW);
    glfwSetScrollCallback(window, ScrollCallbackGLFW);

    LOG_INFO("Window created: ", width, "x", height);
    return true;
}

void Window::Shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool Window::ShouldClose() const {
    return window && glfwWindowShouldClose(window);
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    if (window) {
        glfwSwapBuffers(window);
    }
}

void Window::SetTitle(const std::string& newTitle) {
    title = newTitle;
    if (window) {
        glfwSetWindowTitle(window, title.c_str());
    }
}

void Window::SetVSync(bool enabled) {
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::SetFullscreen(bool enabled) {
    if (!window || isFullscreen == enabled) return;

    if (enabled) {
        // Save current windowed position and size
        glfwGetWindowPos(window, &windowedX, &windowedY);
        glfwGetWindowSize(window, &windowedWidth, &windowedHeight);

        // Switch to fullscreen
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

        width = mode->width;
        height = mode->height;
        isFullscreen = true;

        LOG_INFO("Switched to fullscreen: ", width, "x", height);
    } else {
        // Restore windowed mode
        glfwSetWindowMonitor(window, nullptr, windowedX, windowedY, windowedWidth, windowedHeight, 0);

        width = windowedWidth;
        height = windowedHeight;
        isFullscreen = false;

        LOG_INFO("Switched to windowed: ", width, "x", height);
    }
}

void Window::ToggleFullscreen() {
    SetFullscreen(!isFullscreen);
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->width = width;
        self->height = height;
        if (self->resizeCallback) {
            self->resizeCallback(width, height);
        }
    }
}

void Window::KeyCallbackGLFW(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->keyCallback) {
        self->keyCallback(key, scancode, action, mods);
    }
}

void Window::MouseButtonCallbackGLFW(GLFWwindow* window, int button, int action, int mods) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->mouseButtonCallback) {
        self->mouseButtonCallback(button, action, mods);
    }
}

void Window::CursorPosCallbackGLFW(GLFWwindow* window, double xpos, double ypos) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->mouseMoveCallback) {
        self->mouseMoveCallback(xpos, ypos);
    }
}

void Window::ScrollCallbackGLFW(GLFWwindow* window, double xoffset, double yoffset) {
    Window* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self && self->scrollCallback) {
        self->scrollCallback(xoffset, yoffset);
    }
}

} // namespace mo3d
