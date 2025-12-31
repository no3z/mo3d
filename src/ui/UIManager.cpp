#include "UIManager.h"
#include "../utils/Logger.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace mo3d {

UIManager::UIManager()
    : showDemoWindow(false)
    , showMidiPanel(true)
    , showMappingPanel(true)
    , showScenePanel(true)
    , showPerformancePanel(true)
{
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

void UIManager::RenderMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load Scene")) {}
            if (ImGui::MenuItem("Save Scene")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("MIDI Panel", nullptr, &showMidiPanel);
            ImGui::MenuItem("Mapping Editor", nullptr, &showMappingPanel);
            ImGui::MenuItem("Scene Panel", nullptr, &showScenePanel);
            ImGui::MenuItem("Performance", nullptr, &showPerformancePanel);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", nullptr, &showDemoWindow);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }
}

void UIManager::RenderMidiPanel(MidiInput* midiInput) {
    if (!showMidiPanel || !midiInput) return;

    ImGui::Begin("MIDI Input", &showMidiPanel);

    if (midiInput->IsPortOpen()) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected: %s", midiInput->GetCurrentPortName().c_str());
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Connected");
    }

    ImGui::Separator();

    auto ports = midiInput->GetAvailablePorts();
    for (size_t i = 0; i < ports.size(); i++) {
        if (ImGui::Button(ports[i].c_str())) {
            midiInput->OpenPort(i);
        }
    }

    ImGui::End();
}

void UIManager::RenderMappingEditor(MidiMapping* mapping) {
    if (!showMappingPanel || !mapping) return;

    ImGui::Begin("MIDI Mapping", &showMappingPanel);

    ImGui::Text("MIDI Mappings Editor");
    ImGui::Separator();

    auto mappings = mapping->GetAllMappings();
    for (auto* m : mappings) {
        ImGui::PushID(m->id.c_str());

        bool enabled = m->enabled;
        if (ImGui::Checkbox("##enabled", &enabled)) {
            mapping->EnableMapping(m->id, enabled);
        }

        ImGui::SameLine();
        ImGui::Text("%s", m->name.c_str());
        ImGui::SameLine();
        ImGui::Text("[Ch:%d Note/CC:%d]", m->channel, m->noteOrCC);

        ImGui::PopID();
    }

    ImGui::End();
}

void UIManager::RenderScenePanel(Scene* scene) {
    if (!showScenePanel || !scene) return;

    ImGui::Begin("Scene", &showScenePanel);

    ImGui::Text("Objects: %zu", scene->GetObjectCount());
    ImGui::Separator();

    auto objects = scene->GetAllObjects();
    for (auto& obj : objects) {
        if (ImGui::TreeNode(obj->GetName().c_str())) {
            auto pos = obj->transform.GetPosition();
            ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);

            bool enabled = obj->IsEnabled();
            if (ImGui::Checkbox("Enabled", &enabled)) {
                obj->SetEnabled(enabled);
            }

            ImGui::TreePop();
        }
    }

    ImGui::End();
}

void UIManager::RenderPerformancePanel() {
    if (!showPerformancePanel) return;

    ImGui::Begin("Performance", &showPerformancePanel);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

    ImGui::End();
}

} // namespace mo3d
