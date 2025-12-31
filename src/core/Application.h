#pragma once

#include "Window.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiMapping.h"
#include "../scene/Scene.h"
#include "../render/Renderer.h"
#include "../ui/UIManager.h"
#include <memory>

namespace mo3d {

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

    bool running;

    void Update(float deltaTime);
    void Render();
    void HandleInput();

    void SetupScene();
    void SetupMidiMappings();
    void OnMidiEvent(const MidiEvent& event);
};

} // namespace mo3d
