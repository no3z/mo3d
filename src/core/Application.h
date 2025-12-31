#pragma once

#include "Window.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiMapping.h"
#include "../scene/Scene.h"
#include "../render/Renderer.h"
#include "../ui/UIManager.h"
#include <memory>

namespace mo3d {

class ProjectManager;

class Application {
public:
    Application();
    ~Application();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<MidiInput> midiInput;
    std::unique_ptr<MidiMapping> midiMapping;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<UIManager> uiManager;
    std::unique_ptr<ProjectManager> projectManager;

    bool running;
    bool showProjectPanel;
    bool showSettingsPanel;

    void Update(float deltaTime);
    void Render();
    void HandleInput();
    void HandleKeyboardShortcuts();

    void SetupScene();
    void SetupMidiMappings();
    void OnMidiEvent(const MidiEvent& event);

    void SaveProject();
    void LoadProject(const std::string& path);
    void AutoConnectMidi();
};

} // namespace mo3d
