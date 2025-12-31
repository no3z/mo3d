#pragma once

#include <cstdint>
#include <string>

namespace mo3d {

enum class MidiMessageType {
    NoteOff = 0x80,
    NoteOn = 0x90,
    PolyAftertouch = 0xA0,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
    ChannelAftertouch = 0xD0,
    PitchBend = 0xE0,
    SystemMessage = 0xF0,
    Unknown = 0x00
};

struct MidiEvent {
    MidiMessageType type;
    uint8_t channel;
    uint8_t data1;  // Note number or CC number
    uint8_t data2;  // Velocity or CC value
    double timestamp;

    MidiEvent()
        : type(MidiMessageType::Unknown)
        , channel(0)
        , data1(0)
        , data2(0)
        , timestamp(0.0)
    {}

    MidiEvent(MidiMessageType t, uint8_t ch, uint8_t d1, uint8_t d2, double ts = 0.0)
        : type(t)
        , channel(ch)
        , data1(d1)
        , data2(d2)
        , timestamp(ts)
    {}

    static MidiEvent FromRawBytes(const uint8_t* message, size_t size, double timestamp = 0.0) {
        if (size < 1) return MidiEvent();

        uint8_t status = message[0];
        MidiMessageType type = static_cast<MidiMessageType>(status & 0xF0);
        uint8_t channel = status & 0x0F;

        uint8_t data1 = size > 1 ? message[1] : 0;
        uint8_t data2 = size > 2 ? message[2] : 0;

        return MidiEvent(type, channel, data1, data2, timestamp);
    }

    bool IsNoteOn() const {
        return type == MidiMessageType::NoteOn && data2 > 0;
    }

    bool IsNoteOff() const {
        return type == MidiMessageType::NoteOff || (type == MidiMessageType::NoteOn && data2 == 0);
    }

    bool IsCC() const {
        return type == MidiMessageType::ControlChange;
    }

    uint8_t GetNote() const { return data1; }
    uint8_t GetVelocity() const { return data2; }
    uint8_t GetCCNumber() const { return data1; }
    uint8_t GetCCValue() const { return data2; }

    float GetNormalizedVelocity() const {
        return static_cast<float>(data2) / 127.0f;
    }

    float GetNormalizedCCValue() const {
        return static_cast<float>(data2) / 127.0f;
    }

    std::string ToString() const {
        std::string result = "MIDI [Ch:" + std::to_string(channel) + "] ";
        switch (type) {
            case MidiMessageType::NoteOn:
                result += "NoteOn  Note:" + std::to_string(data1) + " Vel:" + std::to_string(data2);
                break;
            case MidiMessageType::NoteOff:
                result += "NoteOff Note:" + std::to_string(data1);
                break;
            case MidiMessageType::ControlChange:
                result += "CC      Num:" + std::to_string(data1) + " Val:" + std::to_string(data2);
                break;
            case MidiMessageType::PitchBend:
                result += "PitchBend";
                break;
            default:
                result += "Other";
                break;
        }
        return result;
    }
};

} // namespace mo3d
