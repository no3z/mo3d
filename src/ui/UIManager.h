#pragma once

#include "FileDialog.h"
#include <memory>

struct GLFWwindow;

namespace mo3d {

class MidiInput;
class MidiMapping;
class Scene;
class ProjectManager;
class Window;
class Renderer;

class UIManager {
public:
    UIManager();
    ~UIManager();

    bool Initialize(GLFWwindow* window);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void RenderMainMenu(bool* showProjectPanel, bool* showSettingsPanel);
    void RenderMidiPanel(MidiInput* midiInput, bool* show);
    void RenderMappingEditor(MidiMapping* mapping, bool* show);
    void RenderScenePanel(Scene* scene, bool* show);
    void RenderPerformancePanel(bool* show);
    void RenderProjectPanel(ProjectManager* projectManager, Scene* scene, MidiMapping* mapping, bool* show);
    void RenderSettingsPanel(Window* window, Renderer* renderer, bool* show);

private:
    bool showDemoWindow;

    // File dialogs
    FileDialog projectOpenDialog;
    FileDialog projectSaveDialog;
    FileDialog mappingImportDialog;
    FileDialog mappingExportDialog;

    // For input buffers
    std::string projectNameBuffer;
    std::string projectAuthorBuffer;
    std::string projectDescBuffer;
    std::string savePathBuffer;
};

} // namespace mo3d
