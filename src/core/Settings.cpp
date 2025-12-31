#include "Settings.h"
#include "../utils/Logger.h"
#include "../utils/FileIO.h"

namespace mo3d {

bool Settings::LoadFromFile(const std::string& filename) {
    try {
        std::string content = FileIO::ReadTextFile(filename);
        if (content.empty()) {
            LOG_WARN("Settings file not found, using defaults: ", filename);
            return false;
        }

        nlohmann::json json = nlohmann::json::parse(content);
        FromJson(json);

        LOG_INFO("Settings loaded from: ", filename);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load settings: ", e.what());
        return false;
    }
}

bool Settings::SaveToFile(const std::string& filename) const {
    try {
        nlohmann::json json = ToJson();
        std::string content = json.dump(2);

        if (!FileIO::WriteTextFile(filename, content)) {
            LOG_ERROR("Failed to write settings file: ", filename);
            return false;
        }

        LOG_INFO("Settings saved to: ", filename);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save settings: ", e.what());
        return false;
    }
}

nlohmann::json Settings::ToJson() const {
    nlohmann::json json;

    // Window settings
    json["window"]["width"] = window.width;
    json["window"]["height"] = window.height;
    json["window"]["fullscreen"] = window.fullscreen;
    json["window"]["vsync"] = window.vsync;
    json["window"]["msaaSamples"] = window.msaaSamples;

    // MIDI settings
    json["midi"]["lastInputPort"] = midi.lastInputPort;
    json["midi"]["lastInputPortIndex"] = midi.lastInputPortIndex;
    json["midi"]["autoConnect"] = midi.autoConnect;
    json["midi"]["virtualPortName"] = midi.virtualPortName;

    // Render settings
    json["render"]["wireframe"] = render.wireframe;
    json["render"]["clearColorR"] = render.clearColorR;
    json["render"]["clearColorG"] = render.clearColorG;
    json["render"]["clearColorB"] = render.clearColorB;
    json["render"]["showGrid"] = render.showGrid;
    json["render"]["showFPS"] = render.showFPS;

    // UI settings
    json["ui"]["showMidiPanel"] = ui.showMidiPanel;
    json["ui"]["showMappingPanel"] = ui.showMappingPanel;
    json["ui"]["showScenePanel"] = ui.showScenePanel;
    json["ui"]["showPerformancePanel"] = ui.showPerformancePanel;
    json["ui"]["showSettingsPanel"] = ui.showSettingsPanel;
    json["ui"]["showProjectPanel"] = ui.showProjectPanel;
    json["ui"]["uiScale"] = ui.uiScale;

    return json;
}

void Settings::FromJson(const nlohmann::json& json) {
    try {
        // Window settings
        if (json.contains("window")) {
            auto& w = json["window"];
            if (w.contains("width")) window.width = w["width"];
            if (w.contains("height")) window.height = w["height"];
            if (w.contains("fullscreen")) window.fullscreen = w["fullscreen"];
            if (w.contains("vsync")) window.vsync = w["vsync"];
            if (w.contains("msaaSamples")) window.msaaSamples = w["msaaSamples"];
        }

        // MIDI settings
        if (json.contains("midi")) {
            auto& m = json["midi"];
            if (m.contains("lastInputPort")) midi.lastInputPort = m["lastInputPort"];
            if (m.contains("lastInputPortIndex")) midi.lastInputPortIndex = m["lastInputPortIndex"];
            if (m.contains("autoConnect")) midi.autoConnect = m["autoConnect"];
            if (m.contains("virtualPortName")) midi.virtualPortName = m["virtualPortName"];
        }

        // Render settings
        if (json.contains("render")) {
            auto& r = json["render"];
            if (r.contains("wireframe")) render.wireframe = r["wireframe"];
            if (r.contains("clearColorR")) render.clearColorR = r["clearColorR"];
            if (r.contains("clearColorG")) render.clearColorG = r["clearColorG"];
            if (r.contains("clearColorB")) render.clearColorB = r["clearColorB"];
            if (r.contains("showGrid")) render.showGrid = r["showGrid"];
            if (r.contains("showFPS")) render.showFPS = r["showFPS"];
        }

        // UI settings
        if (json.contains("ui")) {
            auto& u = json["ui"];
            if (u.contains("showMidiPanel")) ui.showMidiPanel = u["showMidiPanel"];
            if (u.contains("showMappingPanel")) ui.showMappingPanel = u["showMappingPanel"];
            if (u.contains("showScenePanel")) ui.showScenePanel = u["showScenePanel"];
            if (u.contains("showPerformancePanel")) ui.showPerformancePanel = u["showPerformancePanel"];
            if (u.contains("showSettingsPanel")) ui.showSettingsPanel = u["showSettingsPanel"];
            if (u.contains("showProjectPanel")) ui.showProjectPanel = u["showProjectPanel"];
            if (u.contains("uiScale")) ui.uiScale = u["uiScale"];
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error parsing settings JSON: ", e.what());
    }
}

void Settings::Reset() {
    window = WindowSettings();
    midi = MidiSettings();
    render = RenderSettings();
    ui = UISettings();
    LOG_INFO("Settings reset to defaults");
}

} // namespace mo3d
