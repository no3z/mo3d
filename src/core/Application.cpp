#include "Application.h"
#include "Time.h"
#include "Settings.h"
#include "ProjectManager.h"
#include "../utils/Logger.h"
#include "../utils/FileIO.h"
#include "../render/Mesh.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>

namespace mo3d {

Application::Application()
    : running(false)
    , showProjectPanel(false)
    , showSettingsPanel(false)
{
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    LOG_INFO("=== Mo3D - MIDI 3D Visualization Platform ===");
    LOG_INFO("Initializing application...");

    Time::Init();

    // Load settings
    Settings::Instance().LoadFromFile();
    auto& settings = Settings::Instance();

    // Create window with settings
    window = std::make_unique<Window>();
    WindowConfig config;
    config.width = settings.window.width;
    config.height = settings.window.height;
    config.title = "Mo3D - MIDI 3D Visualization";
    config.vsync = settings.window.vsync;
    config.fullscreen = settings.window.fullscreen;
    config.samples = settings.window.msaaSamples;

    if (!window->Initialize(config)) {
        LOG_ERROR("Failed to initialize window");
        return false;
    }

    window->SetResizeCallback([this](int width, int height) {
        auto& settings = Settings::Instance();
        settings.window.width = width;
        settings.window.height = height;

        if (scene && scene->GetMainCamera()) {
            scene->GetMainCamera()->SetAspectRatio((float)width / (float)height);
        }
    });

    // Setup key callback for shortcuts
    window->SetKeyCallback([this](int key, int scancode, int action, int mods) {
        // F11 - Toggle fullscreen
        if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
            window->ToggleFullscreen();
            Settings::Instance().window.fullscreen = window->IsFullscreen();
        }
    });

    renderer = std::make_unique<Renderer>();
    if (!renderer->Initialize()) {
        LOG_ERROR("Failed to initialize renderer");
        return false;
    }

    renderer->SetClearColor(glm::vec3(
        settings.render.clearColorR,
        settings.render.clearColorG,
        settings.render.clearColorB
    ));
    renderer->SetWireframe(settings.render.wireframe);

    midiInput = std::make_unique<MidiInput>();
    if (!midiInput->Initialize()) {
        LOG_ERROR("Failed to initialize MIDI input");
        return false;
    }

    midiInput->SetCallback([this](const MidiEvent& event) {
        OnMidiEvent(event);
    });

    // Auto-connect MIDI if configured
    AutoConnectMidi();

    midiMapping = std::make_unique<MidiMapping>();

    scene = std::make_unique<Scene>("MainScene");
    scene->GetMainCamera()->SetPerspective(45.0f, window->GetAspectRatio(), 0.1f, 1000.0f);
    scene->GetMainCamera()->transform.SetPosition(0.0f, 2.0f, 8.0f);
    scene->GetMainCamera()->LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

    uiManager = std::make_unique<UIManager>();
    if (!uiManager->Initialize(window->GetNativeWindow())) {
        LOG_ERROR("Failed to initialize UI manager");
        return false;
    }

    projectManager = std::make_unique<ProjectManager>();
    projectManager->NewProject("Default Project");

    SetupScene();
    SetupMidiMappings();

    running = true;
    LOG_INFO("Application initialized successfully");
    LOG_INFO("Press F11 for fullscreen, ESC to exit");
    return true;
}

void Application::Run() {
    while (!window->ShouldClose() && running) {
        Time::Update();
        float deltaTime = Time::DeltaTime();

        HandleInput();
        HandleKeyboardShortcuts();
        Update(deltaTime);
        Render();

        window->PollEvents();
        window->SwapBuffers();
    }
}

void Application::Shutdown() {
    LOG_INFO("Shutting down application...");

    // Save settings on exit
    Settings::Instance().SaveToFile();

    uiManager.reset();
    projectManager.reset();
    scene.reset();
    midiMapping.reset();
    midiInput.reset();
    renderer.reset();
    window.reset();

    LOG_INFO("Application shutdown complete");
}

void Application::Update(float deltaTime) {
    if (scene) {
        scene->Update(deltaTime);
    }
}

void Application::Render() {
    renderer->BeginFrame();

    if (scene && scene->GetMainCamera()) {
        renderer->RenderScene(scene.get(), scene->GetMainCamera().get());
    }

    auto& settings = Settings::Instance();

    uiManager->BeginFrame();
    uiManager->RenderMainMenu(&showProjectPanel, &showSettingsPanel);
    uiManager->RenderMidiPanel(midiInput.get(), &settings.ui.showMidiPanel);
    uiManager->RenderMappingEditor(midiMapping.get(), &settings.ui.showMappingPanel);
    uiManager->RenderScenePanel(scene.get(), &settings.ui.showScenePanel);
    uiManager->RenderPerformancePanel(&settings.ui.showPerformancePanel);

    if (showProjectPanel) {
        uiManager->RenderProjectPanel(projectManager.get(), scene.get(), midiMapping.get(), &showProjectPanel);
    }

    if (showSettingsPanel) {
        uiManager->RenderSettingsPanel(window.get(), renderer.get(), &showSettingsPanel);
    }

    uiManager->EndFrame();

    renderer->EndFrame();
}

void Application::HandleInput() {
    if (glfwGetKey(window->GetNativeWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        running = false;
    }
}

void Application::HandleKeyboardShortcuts() {
    GLFWwindow* nativeWindow = window->GetNativeWindow();

    // Check for Ctrl key
    bool ctrlPressed = glfwGetKey(nativeWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                      glfwGetKey(nativeWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;

    bool shiftPressed = glfwGetKey(nativeWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                       glfwGetKey(nativeWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    static bool nWasPressed = false;
    static bool oWasPressed = false;
    static bool sWasPressed = false;

    // Ctrl+N - New Project
    if (ctrlPressed && glfwGetKey(nativeWindow, GLFW_KEY_N) == GLFW_PRESS) {
        if (!nWasPressed) {
            projectManager->NewProject("New Project");
            scene->Clear();
            midiMapping->ClearMappings();
            SetupScene();
            SetupMidiMappings();
            nWasPressed = true;
        }
    } else {
        nWasPressed = false;
    }

    // Ctrl+O - Open Project
    if (ctrlPressed && glfwGetKey(nativeWindow, GLFW_KEY_O) == GLFW_PRESS) {
        if (!oWasPressed) {
            showProjectPanel = true;
            oWasPressed = true;
        }
    } else {
        oWasPressed = false;
    }

    // Ctrl+S - Save Project
    if (ctrlPressed && glfwGetKey(nativeWindow, GLFW_KEY_S) == GLFW_PRESS) {
        if (!sWasPressed) {
            SaveProject();
            sWasPressed = true;
        }
    } else {
        sWasPressed = false;
    }
}

void Application::SetupScene() {
    LOG_INFO("Setting up demo scene...");

    for (int i = 0; i < 12; i++) {
        auto obj = scene->CreateObject("Cube_" + std::to_string(i));
        obj->SetMesh(std::shared_ptr<Mesh>(Mesh::CreateCube()));

        float angle = (i / 12.0f) * glm::two_pi<float>();
        float radius = 4.0f;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        obj->transform.SetPosition(x, 0.0f, z);
        obj->transform.SetScale(0.5f);

        float hue = i / 12.0f;
        glm::vec3 color(
            0.5f + 0.5f * cos(hue * 6.28f),
            0.5f + 0.5f * cos((hue + 0.33f) * 6.28f),
            0.5f + 0.5f * cos((hue + 0.67f) * 6.28f)
        );
        obj->renderProps.color = glm::vec4(color, 1.0f);
    }

    LOG_INFO("Created ", scene->GetObjectCount(), " objects");
}

void Application::SetupMidiMappings() {
    LOG_INFO("Setting up MIDI mappings...");

    for (int note = 60; note < 72; note++) {
        int objectIndex = note - 60;
        std::string objectName = "Cube_" + std::to_string(objectIndex);
        auto object = scene->FindObject(objectName);

        if (!object) continue;

        midiMapping->AddFloatMapping(
            "Scale_" + objectName,
            MidiTriggerType::Velocity,
            0,
            note,
            [object](float value) {
                float scale = 0.2f + value * 2.0f;
                object->transform.SetScale(scale);
            },
            MappingMode::Direct,
            ParameterRange{0.2f, 2.0f, 0.5f}
        );

        midiMapping->AddFloatMapping(
            "Opacity_" + objectName,
            MidiTriggerType::NoteOn,
            0,
            note,
            [object](float value) {
                object->FadeIn(0.1f);
            },
            MappingMode::Trigger
        );

        midiMapping->AddTriggerMapping(
            "FadeOut_" + objectName,
            MidiTriggerType::NoteOff,
            0,
            note,
            [object]() {
                object->FadeOut(0.5f);
            }
        );
    }

    LOG_INFO("Created ", midiMapping->GetAllMappings().size(), " MIDI mappings");
}

void Application::OnMidiEvent(const MidiEvent& event) {
    LOG_DEBUG(event.ToString());
    midiMapping->ProcessMidiEvent(event);
}

void Application::SaveProject() {
    std::string path = projectManager->GetCurrentProjectPath();
    if (path.empty()) {
        path = "project.mo3d";
    }

    auto json = projectManager->SerializeProject(scene.get(), midiMapping.get());
    std::string content = json.dump(2);

    if (FileIO::WriteTextFile(path, content)) {
        projectManager->SaveProject(path);
        LOG_INFO("Project saved successfully");
    } else {
        LOG_ERROR("Failed to save project");
    }
}

void Application::LoadProject(const std::string& path) {
    std::string content = FileIO::ReadTextFile(path);
    if (content.empty()) {
        LOG_ERROR("Failed to load project: ", path);
        return;
    }

    try {
        nlohmann::json json = nlohmann::json::parse(content);

        if (projectManager->LoadProject(path)) {
            projectManager->DeserializeProject(json, scene.get(), midiMapping.get());

            // Re-setup MIDI mappings (callbacks need to be re-registered)
            SetupMidiMappings();

            LOG_INFO("Project loaded successfully");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse project: ", e.what());
    }
}

void Application::AutoConnectMidi() {
    auto& settings = Settings::Instance();

    if (!settings.midi.autoConnect) {
        return;
    }

    auto ports = midiInput->GetAvailablePorts();
    if (ports.empty()) {
        LOG_WARN("No MIDI ports available for auto-connect");
        return;
    }

    // Try to connect to last used port
    if (settings.midi.lastInputPortIndex >= 0 &&
        settings.midi.lastInputPortIndex < static_cast<int>(ports.size())) {

        if (midiInput->OpenPort(settings.midi.lastInputPortIndex)) {
            LOG_INFO("Auto-connected to MIDI port: ", settings.midi.lastInputPort);
            return;
        }
    }

    // Fallback: connect to first available port
    if (midiInput->OpenPort(0)) {
        LOG_INFO("Auto-connected to first MIDI port: ", ports[0]);
        settings.midi.lastInputPort = ports[0];
        settings.midi.lastInputPortIndex = 0;
    }
}

} // namespace mo3d
