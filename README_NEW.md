# Mo3D - MIDI-Driven 3D Visualization Platform

**Mo3D** is a professional, real-time 3D visualization platform driven by MIDI input. Map MIDI notes and CC values to any object parameter (position, rotation, scale, color, opacity, etc.) to create stunning audio-reactive visuals.

## Features

### Core Capabilities
- **Real-time MIDI Input** - Connect any MIDI controller or DAW
- **Flexible Parameter Mapping** - Map MIDI events to any object property
- **3D Object System** - Cubes, spheres, planes, custom meshes
- **2D Sprite Support** - Alpha-blended textures and sprites
- **Advanced Camera** - Perspective/orthographic with orbit controls
- **Modern Rendering** - OpenGL 4.1+ with shader support
- **ImGui Interface** - Real-time mapping and scene editing

### MIDI Mapping System
- **Trigger Types**: NoteOn, NoteOff, CC, Velocity
- **Mapping Modes**:
  - **Direct** - Map values directly (0-127 → min-max)
  - **Toggle** - Toggle on/off states
  - **Trigger** - Fire events on MIDI input
  - **Momentary** - Active while note held
  - **Increment/Decrement** - Step through values

### Object Properties (All MIDI-Mappable)
- Position (X, Y, Z)
- Rotation (Euler or Quaternion)
- Scale (Uniform or Per-Axis)
- Color (RGBA)
- Opacity with fade effects
- Custom shader parameters

## Architecture

```
Mo3D/
├── src/
│   ├── core/          # Application, Window, Time
│   ├── midi/          # MIDI input and mapping system
│   ├── scene/         # Scene graph, objects, camera
│   ├── render/        # OpenGL renderer, shaders, meshes
│   ├── ui/            # ImGui interface
│   └── utils/         # Logging, file I/O
├── shaders/           # GLSL shaders
├── assets/            # Textures, fonts, models
└── CMakeLists.txt     # Build configuration
```

## Building

### Dependencies
Mo3D uses **vcpkg** for dependency management. The following libraries are required:

- **GLFW3** - Windowing and input
- **GLM** - Mathematics library
- **RtMidi** - Cross-platform MIDI I/O
- **ImGui** - Immediate mode GUI
- **nlohmann-json** - JSON serialization
- **FreeType** - Font rendering
- **stb_image** - Image loading
- **Vulkan SDK** (optional) - Advanced rendering

### Build Instructions

#### 1. Install vcpkg (if not already installed)
```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # On Linux/macOS
./bootstrap-vcpkg.bat  # On Windows
export VCPKG_ROOT=$(pwd)  # Add to ~/.bashrc or ~/.zshrc
```

#### 2. Build Mo3D
```bash
cd mo3d
mkdir build && cd build

# Configure with vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . --config Release

# Run
./mo3d
```

### Platform-Specific Notes

#### Linux
```bash
# Install system dependencies (Ubuntu/Debian)
sudo apt-get install build-essential cmake pkg-config libgl1-mesa-dev libglu1-mesa-dev

# ALSA (for MIDI on Linux)
sudo apt-get install libasound2-dev
```

#### macOS
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Use Homebrew for additional tools
brew install cmake pkg-config
```

#### Windows
- Install Visual Studio 2019 or later
- Use CMake GUI or command line with Visual Studio generator
- vcpkg will handle all dependencies automatically

## Quick Start

### 1. Connect MIDI Device
Launch Mo3D and open the **MIDI Input** panel. Your connected MIDI devices will appear in the list. Click on a device to connect.

### 2. Understand the Demo Scene
The default scene creates 12 cubes in a circle, each mapped to MIDI notes 60-71 (C4-B4):
- **Note On** → Fade in cube
- **Note Off** → Fade out cube
- **Velocity** → Scale cube (higher velocity = larger)

### 3. Create Custom Mappings

```cpp
// Example: Map MIDI CC 1 (Mod Wheel) to camera FOV
midiMapping->AddFloatMapping(
    "Camera FOV",
    MidiTriggerType::CCValue,
    0,                          // Channel 0
    1,                          // CC number 1
    [camera](float value) {
        camera->SetFOV(30.0f + value * 60.0f);  // 30-90 degrees
    },
    MappingMode::Direct,
    ParameterRange{30.0f, 90.0f}
);

// Example: Map Note C3 to trigger new object spawn
midiMapping->AddTriggerMapping(
    "Spawn Object",
    MidiTriggerType::NoteOn,
    0,                          // Channel 0
    48,                         // C3
    [scene]() {
        auto obj = scene->CreateObject("NewCube");
        obj->SetMesh(std::shared_ptr<Mesh>(Mesh::CreateCube()));
    }
);
```

### 4. Modify Objects in Real-time

```cpp
// Create animated rotating cube
auto cube = scene->CreateObject("RotatingCube");
cube->SetMesh(std::shared_ptr<Mesh>(Mesh::CreateCube()));
cube->transform.SetPosition(0.0f, 2.0f, 0.0f);

// Add rotation animation
cube->RotateOverTime(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);

// Pulse opacity
cube->PulseOpacity(2.0f, 0.3f, 1.0f);
```

## Development Roadmap

### Phase 1: Foundation ✅ (Complete)
- [x] Core application architecture
- [x] MIDI input system with RtMidi
- [x] Flexible parameter mapping
- [x] 3D scene graph
- [x] OpenGL renderer
- [x] ImGui interface

### Phase 2: Advanced Features (In Progress)
- [ ] FreeType font rendering
- [ ] ShaderToy shader loader
- [ ] Particle system
- [ ] Post-processing effects
- [ ] Scene save/load system
- [ ] Preset management

### Phase 3: Vulkan & Performance
- [ ] Vulkan renderer backend
- [ ] GPU-accelerated particles
- [ ] Multi-threaded rendering
- [ ] Advanced lighting (PBR)

### Phase 4: Advanced Visualization
- [ ] Audio analysis (FFT, beat detection)
- [ ] Video texture support
- [ ] OSC protocol support
- [ ] DMX lighting integration
- [ ] Live coding interface

## Example Use Cases

- **Live Performance** - VJs and audio-visual artists
- **Music Production** - Real-time visualization in DAWs
- **Interactive Installations** - Museums, galleries, events
- **Education** - Teaching MIDI, 3D graphics, real-time systems
- **Creative Coding** - Algorithmic art and generative visuals

## Technical Details

### MIDI Mapping Pipeline
```
MIDI Device → RtMidi → MidiEvent → MidiMapping → ParameterCallback → Object Update
```

### Rendering Pipeline
```
Scene Update → Camera Update → Renderer BeginFrame → Object Culling →
Shader Bind → Uniforms Set → Mesh Draw → ImGui Render → Swap Buffers
```

### Object Update Cycle
```cpp
void Application::Update(float deltaTime) {
    // 1. Process MIDI events
    auto events = midiInput->GetQueuedEvents();
    for (auto& event : events) {
        midiMapping->ProcessMidiEvent(event);
    }

    // 2. Update scene
    scene->Update(deltaTime);  // Calls Update() on all objects

    // 3. Update camera
    camera->Update(deltaTime);
}
```

## Performance Tips

1. **Reduce Object Count** - Keep active objects under 1000 for 60 FPS
2. **Use Instancing** - For many identical objects
3. **Optimize Shaders** - Avoid complex fragment shaders
4. **Cull Invisible Objects** - Implement frustum culling
5. **Batch Draw Calls** - Group objects by material

## Contributing

Contributions are welcome! Areas of interest:
- New geometric primitives (torus, cylinder, etc.)
- Shader effects and post-processing
- Performance optimizations
- Documentation and examples
- Platform-specific fixes

## Credits

**Original Mo3D** (2002) - University project by the original author
**Mo3D 2.0** (2025) - Complete rewrite in modern C++ with advanced features

## License

MIT License - See LICENSE file for details

## Support

- GitHub Issues: [Report bugs and request features](https://github.com/yourusername/mo3d/issues)
- Documentation: [Full API documentation](https://mo3d.readthedocs.io)
- Discord: [Join the community](https://discord.gg/mo3d)

---

**Mo3D** - Transform MIDI into mesmerizing 3D visuals. 🎹 → 🎨 → 🚀
