#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace mo3d {

struct WindowSettings {
    int width = 1920;
    int height = 1080;
    bool fullscreen = false;
    bool vsync = true;
    int msaaSamples = 4;
};

struct MidiSettings {
    std::string lastInputPort;
    int lastInputPortIndex = -1;
    bool autoConnect = false;
    std::string virtualPortName = "Mo3D Virtual Input";
};

struct RenderSettings {
    bool wireframe = false;
    float clearColorR = 0.1f;
    float clearColorG = 0.1f;
    float clearColorB = 0.1f;
    bool showGrid = false;
    bool showFPS = true;
};

struct UISettings {
    bool showMidiPanel = true;
    bool showMappingPanel = true;
    bool showScenePanel = true;
    bool showPerformancePanel = true;
    bool showSettingsPanel = false;
    bool showProjectPanel = false;
    float uiScale = 1.0f;
};

class Settings {
public:
    WindowSettings window;
    MidiSettings midi;
    RenderSettings render;
    UISettings ui;

    static Settings& Instance() {
        static Settings instance;
        return instance;
    }

    bool LoadFromFile(const std::string& filename = "mo3d_settings.json");
    bool SaveToFile(const std::string& filename = "mo3d_settings.json") const;

    nlohmann::json ToJson() const;
    void FromJson(const nlohmann::json& json);

    void Reset();

private:
    Settings() = default;
};

} // namespace mo3d
