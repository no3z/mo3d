#pragma once

#include <memory>

struct GLFWwindow;

namespace mo3d {

class MidiInput;
class MidiMapping;
class Scene;

class UIManager {
public:
    UIManager();
    ~UIManager();

    bool Initialize(GLFWwindow* window);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void RenderMainMenu();
    void RenderMidiPanel(MidiInput* midiInput);
    void RenderMappingEditor(MidiMapping* mapping);
    void RenderScenePanel(Scene* scene);
    void RenderPerformancePanel();

private:
    bool showDemoWindow;
    bool showMidiPanel;
    bool showMappingPanel;
    bool showScenePanel;
    bool showPerformancePanel;
};

} // namespace mo3d
