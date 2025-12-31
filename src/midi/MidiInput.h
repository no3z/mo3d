#pragma once

#include "MidiEvent.h"
#include <RtMidi.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>

namespace mo3d {

class MidiInput {
public:
    using MidiCallback = std::function<void(const MidiEvent&)>;

    MidiInput();
    ~MidiInput();

    bool Initialize();
    void Shutdown();

    std::vector<std::string> GetAvailablePorts() const;
    bool OpenPort(unsigned int portNumber);
    bool OpenVirtualPort(const std::string& portName = "Mo3D Virtual Input");
    void ClosePort();

    bool IsPortOpen() const { return portOpen; }
    std::string GetCurrentPortName() const { return currentPortName; }

    void SetCallback(MidiCallback callback) { userCallback = callback; }

    void ProcessEvents();

    std::vector<MidiEvent> GetQueuedEvents();
    void ClearQueue();

private:
    std::unique_ptr<RtMidiIn> midiIn;
    bool portOpen;
    std::string currentPortName;
    MidiCallback userCallback;

    std::mutex queueMutex;
    std::queue<MidiEvent> eventQueue;

    static void StaticMidiCallback(double timeStamp, std::vector<unsigned char>* message, void* userData);
    void HandleMidiMessage(double timeStamp, std::vector<unsigned char>* message);
};

} // namespace mo3d
