#include "MidiInput.h"
#include "../utils/Logger.h"

namespace mo3d {

MidiInput::MidiInput()
    : midiIn(nullptr)
    , portOpen(false)
{
}

MidiInput::~MidiInput() {
    Shutdown();
}

bool MidiInput::Initialize() {
    try {
        midiIn = std::make_unique<RtMidiIn>();
        LOG_INFO("MIDI Input initialized successfully");
        return true;
    } catch (RtMidiError& error) {
        LOG_ERROR("Failed to initialize MIDI Input: ", error.getMessage());
        return false;
    }
}

void MidiInput::Shutdown() {
    ClosePort();
    midiIn.reset();
}

std::vector<std::string> MidiInput::GetAvailablePorts() const {
    std::vector<std::string> ports;
    if (!midiIn) return ports;

    unsigned int portCount = midiIn->getPortCount();
    for (unsigned int i = 0; i < portCount; i++) {
        try {
            ports.push_back(midiIn->getPortName(i));
        } catch (RtMidiError& error) {
            LOG_ERROR("Error getting port name: ", error.getMessage());
        }
    }
    return ports;
}

bool MidiInput::OpenPort(unsigned int portNumber) {
    if (!midiIn) {
        LOG_ERROR("MIDI Input not initialized");
        return false;
    }

    ClosePort();

    try {
        if (portNumber >= midiIn->getPortCount()) {
            LOG_ERROR("Invalid port number: ", portNumber);
            return false;
        }

        midiIn->openPort(portNumber);
        midiIn->setCallback(&MidiInput::StaticMidiCallback, this);
        midiIn->ignoreTypes(false, false, false);  // Don't ignore sysex, timing, or active sensing

        currentPortName = midiIn->getPortName(portNumber);
        portOpen = true;

        LOG_INFO("Opened MIDI port: ", currentPortName);
        return true;
    } catch (RtMidiError& error) {
        LOG_ERROR("Failed to open MIDI port: ", error.getMessage());
        return false;
    }
}

bool MidiInput::OpenVirtualPort(const std::string& portName) {
    if (!midiIn) {
        LOG_ERROR("MIDI Input not initialized");
        return false;
    }

    ClosePort();

    try {
        midiIn->openVirtualPort(portName);
        midiIn->setCallback(&MidiInput::StaticMidiCallback, this);
        midiIn->ignoreTypes(false, false, false);

        currentPortName = portName;
        portOpen = true;

        LOG_INFO("Opened virtual MIDI port: ", portName);
        return true;
    } catch (RtMidiError& error) {
        LOG_ERROR("Failed to open virtual MIDI port: ", error.getMessage());
        return false;
    }
}

void MidiInput::ClosePort() {
    if (midiIn && portOpen) {
        midiIn->closePort();
        portOpen = false;
        currentPortName.clear();
        LOG_INFO("Closed MIDI port");
    }
}

void MidiInput::StaticMidiCallback(double timeStamp, std::vector<unsigned char>* message, void* userData) {
    MidiInput* self = static_cast<MidiInput*>(userData);
    self->HandleMidiMessage(timeStamp, message);
}

void MidiInput::HandleMidiMessage(double timeStamp, std::vector<unsigned char>* message) {
    if (!message || message->empty()) return;

    MidiEvent event = MidiEvent::FromRawBytes(message->data(), message->size(), timeStamp);

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        eventQueue.push(event);
    }

    if (userCallback) {
        userCallback(event);
    }
}

void MidiInput::ProcessEvents() {
    // Events are processed in the callback thread
    // This function can be used for additional processing if needed
}

std::vector<MidiEvent> MidiInput::GetQueuedEvents() {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::vector<MidiEvent> events;

    while (!eventQueue.empty()) {
        events.push_back(eventQueue.front());
        eventQueue.pop();
    }

    return events;
}

void MidiInput::ClearQueue() {
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!eventQueue.empty()) {
        eventQueue.pop();
    }
}

} // namespace mo3d
