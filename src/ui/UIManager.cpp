#include "UIManager.h"
#include "../core/Settings.h"
#include "../core/ProjectManager.h"
#include "../core/Window.h"
#include "../render/Renderer.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiMapping.h"
#include "../scene/Scene.h"
#include "../utils/Logger.h"
#include "../utils/FileIO.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace mo3d {

UIManager::UIManager()
    : showDemoWindow(false)
{
    projectNameBuffer.resize(256);
    projectAuthorBuffer.resize(256);
    projectDescBuffer.resize(512);
    savePathBuffer.resize(512);
}

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Initialize(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    LOG_INFO("ImGui initialized");
    return true;
}

void UIManager::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void UIManager::BeginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::RenderMainMenu(bool* showProjectPanel, bool* showSettingsPanel) {
    auto& settings = Settings::Instance();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Project", "Ctrl+N")) {
                *showProjectPanel = true;
            }
            if (ImGui::MenuItem("Open Project", "Ctrl+O")) {
                *showProjectPanel = true;
            }
            if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
                *showProjectPanel = true;
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                *showProjectPanel = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Signal app to close
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Project", nullptr, showProjectPanel);
            ImGui::MenuItem("MIDI Input", nullptr, &settings.ui.showMidiPanel);
            ImGui::MenuItem("MIDI Mapping", nullptr, &settings.ui.showMappingPanel);
            ImGui::MenuItem("Scene", nullptr, &settings.ui.showScenePanel);
            ImGui::MenuItem("Performance", nullptr, &settings.ui.showPerformancePanel);
            ImGui::MenuItem("Settings", nullptr, showSettingsPanel);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", nullptr, &showDemoWindow);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Toggle Fullscreen", "F11")) {
                settings.window.fullscreen = !settings.window.fullscreen;
            }
            if (ImGui::MenuItem("VSync", nullptr, &settings.window.vsync)) {
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {}
            if (ImGui::MenuItem("Documentation")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}

void UIManager::RenderMidiPanel(MidiInput* midiInput, bool* show) {
    if (!*show || !midiInput) return;

    ImGui::Begin("MIDI Input", show);

    if (midiInput->IsPortOpen()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "● Connected");
        ImGui::SameLine();
        ImGui::Text("%s", midiInput->GetCurrentPortName().c_str());

        if (ImGui::Button("Disconnect")) {
            midiInput->ClosePort();
        }
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "○ Not Connected");
    }

    ImGui::Separator();
    ImGui::Text("Available MIDI Ports:");

    auto ports = midiInput->GetAvailablePorts();
    if (ports.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No MIDI ports found");
    } else {
        for (size_t i = 0; i < ports.size(); i++) {
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::Button(ports[i].c_str(), ImVec2(-1, 0))) {
                midiInput->OpenPort(i);
                Settings::Instance().midi.lastInputPort = ports[i];
                Settings::Instance().midi.lastInputPortIndex = static_cast<int>(i);
            }
            ImGui::PopID();
        }
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Virtual Port")) {
        static char virtualPortName[256] = "Mo3D Virtual Input";
        ImGui::InputText("Name", virtualPortName, 256);

        if (ImGui::Button("Create Virtual Port")) {
            midiInput->OpenVirtualPort(virtualPortName);
        }
    }

    ImGui::End();
}

void UIManager::RenderMappingEditor(MidiMapping* mapping, bool* show) {
    if (!*show || !mapping) return;

    ImGui::Begin("MIDI Mapping", show);

    ImGui::Text("Active MIDI Mappings");
    ImGui::Separator();

    if (ImGui::Button("Add Mapping")) {
        // TODO: Open mapping creation dialog
    }

    ImGui::SameLine();
    if (ImGui::Button("Import...")) {
        mappingImportDialog.SetFileExtension(".json");
        mappingImportDialog.Open(FileDialogMode::Open, "Import MIDI Mappings", ".");
    }

    ImGui::SameLine();
    if (ImGui::Button("Export...")) {
        mappingExportDialog.SetFileExtension(".json");
        mappingExportDialog.Open(FileDialogMode::Save, "Export MIDI Mappings", ".");
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear All")) {
        mapping->ClearMappings();
    }

    // Handle import dialog
    if (mappingImportDialog.Render()) {
        std::string path = mappingImportDialog.GetSelectedPath();
        if (mapping->LoadFromFile(path)) {
            LOG_INFO("MIDI mappings imported from: ", path);
        } else {
            LOG_ERROR("Failed to import MIDI mappings");
        }
    }

    // Handle export dialog
    if (mappingExportDialog.Render()) {
        std::string path = mappingExportDialog.GetSelectedPath();
        if (mapping->SaveToFile(path)) {
            LOG_INFO("MIDI mappings exported to: ", path);
        } else {
            LOG_ERROR("Failed to export MIDI mappings");
        }
    }

    ImGui::Separator();

    auto mappings = mapping->GetAllMappings();
    if (mappings.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No mappings configured");
    } else {
        for (auto* m : mappings) {
            ImGui::PushID(m->id.c_str());

            bool enabled = m->enabled;
            if (ImGui::Checkbox("##enabled", &enabled)) {
                mapping->EnableMapping(m->id, enabled);
            }

            ImGui::SameLine();
            ImGui::Text("%s", m->name.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "[Ch:%d Note/CC:%d]", m->channel, m->noteOrCC);

            ImGui::SameLine();
            if (ImGui::SmallButton("X")) {
                mapping->RemoveMapping(m->id);
            }

            ImGui::PopID();
        }
    }

    ImGui::End();
}

void UIManager::RenderScenePanel(Scene* scene, bool* show) {
    if (!*show || !scene) return;

    ImGui::Begin("Scene", show);

    ImGui::Text("Scene: %s", scene->GetName().c_str());
    ImGui::Text("Objects: %zu", scene->GetObjectCount());
    ImGui::Separator();

    auto objects = scene->GetAllObjects();
    for (auto& obj : objects) {
        if (ImGui::TreeNode(obj->GetName().c_str())) {
            auto pos = obj->transform.GetPosition();
            ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);

            auto rot = obj->transform.GetRotationEuler();
            ImGui::Text("Rotation: %.2f, %.2f, %.2f", rot.x, rot.y, rot.z);

            auto scale = obj->transform.GetScale();
            ImGui::Text("Scale: %.2f, %.2f, %.2f", scale.x, scale.y, scale.z);

            bool enabled = obj->IsEnabled();
            if (ImGui::Checkbox("Enabled", &enabled)) {
                obj->SetEnabled(enabled);
            }

            bool visible = obj->renderProps.visible;
            if (ImGui::Checkbox("Visible", &visible)) {
                obj->renderProps.visible = visible;
            }

            float opacity = obj->renderProps.opacity;
            if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f)) {
                obj->renderProps.opacity = opacity;
            }

            ImGui::TreePop();
        }
    }

    ImGui::End();
}

void UIManager::RenderPerformancePanel(bool* show) {
    if (!*show) return;

    ImGui::Begin("Performance", show);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

    static float fpsHistory[90] = {0};
    static int fpsHistoryIndex = 0;
    fpsHistory[fpsHistoryIndex] = ImGui::GetIO().Framerate;
    fpsHistoryIndex = (fpsHistoryIndex + 1) % 90;

    ImGui::PlotLines("##FPS", fpsHistory, 90, 0, nullptr, 0.0f, 120.0f, ImVec2(0, 80));

    ImGui::End();
}

void UIManager::RenderProjectPanel(ProjectManager* projectManager, Scene* scene, MidiMapping* mapping, bool* show) {
    if (!*show || !projectManager) return;

    ImGui::Begin("Project", show);

    auto& info = projectManager->GetProjectInfo();

    ImGui::Text("Project: %s", info.name.c_str());
    if (!info.author.empty()) {
        ImGui::Text("Author: %s", info.author.c_str());
    }

    if (projectManager->HasUnsavedChanges()) {
        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.0f, 1.0f), "● Unsaved changes");
    } else {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "✓ Saved");
    }

    ImGui::Separator();

    if (ImGui::Button("New Project")) {
        projectManager->NewProject("Untitled");
        scene->Clear();
        mapping->ClearMappings();
    }

    ImGui::SameLine();
    if (ImGui::Button("Save Project")) {
        if (!projectManager->GetCurrentProjectPath().empty()) {
            // Save to existing path
            auto json = projectManager->SerializeProject(scene, mapping);
            std::string content = json.dump(2);
            if (FileIO::WriteTextFile(projectManager->GetCurrentProjectPath(), content)) {
                projectManager->MarkSaved();
                LOG_INFO("Project saved");
            }
        } else {
            // Show save dialog for new project
            projectSaveDialog.SetFileExtension(".mo3d");
            projectSaveDialog.Open(FileDialogMode::Save, "Save Project", ".");
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Save As...")) {
        projectSaveDialog.SetFileExtension(".mo3d");
        projectSaveDialog.Open(FileDialogMode::Save, "Save Project As", ".");
    }

    ImGui::SameLine();
    if (ImGui::Button("Open...")) {
        projectOpenDialog.SetFileExtension(".mo3d");
        projectOpenDialog.Open(FileDialogMode::Open, "Open Project", ".");
    }

    // Handle project save dialog
    if (projectSaveDialog.Render()) {
        std::string path = projectSaveDialog.GetSelectedPath();
        auto json = projectManager->SerializeProject(scene, mapping);
        std::string content = json.dump(2);

        if (FileIO::WriteTextFile(path, content)) {
            projectManager->SaveProject(path);
            LOG_INFO("Project saved to: ", path);
        } else {
            LOG_ERROR("Failed to save project");
        }
    }

    // Handle project open dialog
    if (projectOpenDialog.Render()) {
        std::string path = projectOpenDialog.GetSelectedPath();
        std::string content = FileIO::ReadTextFile(path);

        if (!content.empty()) {
            try {
                nlohmann::json json = nlohmann::json::parse(content);

                if (projectManager->LoadProject(path)) {
                    projectManager->DeserializeProject(json, scene, mapping);
                    LOG_INFO("Project loaded from: ", path);
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to load project: ", e.what());
            }
        }
    }

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Project Info")) {
        strcpy(projectNameBuffer.data(), info.name.c_str());
        if (ImGui::InputText("Name", projectNameBuffer.data(), 256)) {
            ProjectInfo newInfo = info;
            newInfo.name = projectNameBuffer.data();
            projectManager->SetProjectInfo(newInfo);
        }

        strcpy(projectAuthorBuffer.data(), info.author.c_str());
        if (ImGui::InputText("Author", projectAuthorBuffer.data(), 256)) {
            ProjectInfo newInfo = info;
            newInfo.author = projectAuthorBuffer.data();
            projectManager->SetProjectInfo(newInfo);
        }

        strcpy(projectDescBuffer.data(), info.description.c_str());
        if (ImGui::InputTextMultiline("Description", projectDescBuffer.data(), 512)) {
            ProjectInfo newInfo = info;
            newInfo.description = projectDescBuffer.data();
            projectManager->SetProjectInfo(newInfo);
        }
    }

    if (ImGui::CollapsingHeader("Recent Projects")) {
        auto recent = projectManager->GetRecentProjects();
        for (const auto& path : recent) {
            if (ImGui::Selectable(path.c_str())) {
                projectManager->LoadProject(path);
                // TODO: Actually load from the path
            }
        }
    }

    ImGui::End();
}

void UIManager::RenderSettingsPanel(Window* window, Renderer* renderer, bool* show) {
    if (!*show) return;

    ImGui::Begin("Settings", show);

    auto& settings = Settings::Instance();

    if (ImGui::CollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Checkbox("Fullscreen", &settings.window.fullscreen)) {
            if (window) {
                window->SetFullscreen(settings.window.fullscreen);
            }
        }

        if (ImGui::Checkbox("VSync", &settings.window.vsync)) {
            if (window) {
                window->SetVSync(settings.window.vsync);
            }
        }

        ImGui::SliderInt("MSAA Samples", &settings.window.msaaSamples, 0, 16);
    }

    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Checkbox("Wireframe", &settings.render.wireframe)) {
            if (renderer) {
                renderer->SetWireframe(settings.render.wireframe);
            }
        }

        ImGui::ColorEdit3("Clear Color", &settings.render.clearColorR);

        if (renderer) {
            renderer->SetClearColor(glm::vec3(
                settings.render.clearColorR,
                settings.render.clearColorG,
                settings.render.clearColorB
            ));
        }
    }

    if (ImGui::CollapsingHeader("MIDI")) {
        ImGui::Checkbox("Auto Connect", &settings.midi.autoConnect);

        char portName[256];
        strcpy(portName, settings.midi.virtualPortName.c_str());
        if (ImGui::InputText("Virtual Port Name", portName, 256)) {
            settings.midi.virtualPortName = portName;
        }
    }

    if (ImGui::CollapsingHeader("UI")) {
        ImGui::SliderFloat("UI Scale", &settings.ui.uiScale, 0.5f, 2.0f);
        ImGui::GetIO().FontGlobalScale = settings.ui.uiScale;
    }

    ImGui::Separator();

    if (ImGui::Button("Save Settings")) {
        settings.SaveToFile();
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Settings")) {
        settings.LoadFromFile();
    }

    ImGui::SameLine();
    if (ImGui::Button("Reset to Defaults")) {
        settings.Reset();
    }

    ImGui::End();
}

} // namespace mo3d
