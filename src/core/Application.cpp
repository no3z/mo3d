#include "Application.h"
#include "Time.h"
#include "../utils/Logger.h"
#include "../render/Mesh.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/constants.hpp>

namespace mo3d {

Application::Application()
    : running(false)
{
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    LOG_INFO("=== Mo3D - MIDI 3D Visualization Platform ===");
    LOG_INFO("Initializing application...");

    Time::Init();

    window = std::make_unique<Window>();
    WindowConfig config;
    config.width = 1920;
    config.height = 1080;
    config.title = "Mo3D - MIDI 3D Visualization";
    config.vsync = true;

    if (!window->Initialize(config)) {
        LOG_ERROR("Failed to initialize window");
        return false;
    }

    window->SetResizeCallback([this](int width, int height) {
        if (scene && scene->GetMainCamera()) {
            scene->GetMainCamera()->SetAspectRatio((float)width / (float)height);
        }
    });

    renderer = std::make_unique<Renderer>();
    if (!renderer->Initialize()) {
        LOG_ERROR("Failed to initialize renderer");
        return false;
    }

    midiInput = std::make_unique<MidiInput>();
    if (!midiInput->Initialize()) {
        LOG_ERROR("Failed to initialize MIDI input");
        return false;
    }

    midiInput->SetCallback([this](const MidiEvent& event) {
        OnMidiEvent(event);
    });

    auto ports = midiInput->GetAvailablePorts();
    if (!ports.empty()) {
        LOG_INFO("Available MIDI ports:");
        for (size_t i = 0; i < ports.size(); i++) {
            LOG_INFO("  [", i, "] ", ports[i]);
        }
    } else {
        LOG_WARN("No MIDI ports found");
    }

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

    SetupScene();
    SetupMidiMappings();

    running = true;
    LOG_INFO("Application initialized successfully");
    return true;
}

void Application::Run() {
    while (!window->ShouldClose() && running) {
        Time::Update();
        float deltaTime = Time::DeltaTime();

        HandleInput();
        Update(deltaTime);
        Render();

        window->PollEvents();
        window->SwapBuffers();
    }
}

void Application::Shutdown() {
    LOG_INFO("Shutting down application...");

    uiManager.reset();
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

    uiManager->BeginFrame();
    uiManager->RenderMainMenu();
    uiManager->RenderMidiPanel(midiInput.get());
    uiManager->RenderMappingEditor(midiMapping.get());
    uiManager->RenderScenePanel(scene.get());
    uiManager->RenderPerformancePanel();
    uiManager->EndFrame();

    renderer->EndFrame();
}

void Application::HandleInput() {
    if (glfwGetKey(window->GetNativeWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        running = false;
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

} // namespace mo3d
