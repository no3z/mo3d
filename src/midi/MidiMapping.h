#pragma once

#include "MidiEvent.h"
#include <nlohmann/json.hpp>
#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

namespace mo3d {

enum class MidiTriggerType {
    NoteOn,
    NoteOff,
    NoteAny,
    CC,
    Velocity,
    CCValue
};

enum class ParameterType {
    Float,
    Vec2,
    Vec3,
    Vec4,
    Color,
    Bool,
    Int
};

enum class MappingMode {
    Direct,         // Direct value mapping (0-127 -> min-max)
    Toggle,         // Toggle on/off
    Trigger,        // Trigger on event
    Momentary,      // Active while note held
    Increment,      // Increment on event
    Decrement       // Decrement on event
};

struct ParameterRange {
    float min = 0.0f;
    float max = 1.0f;
    float defaultValue = 0.0f;

    float Normalize(float value) const {
        return min + (value / 127.0f) * (max - min);
    }

    float Clamp(float value) const {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
};

class MidiMapping {
public:
    using ParameterCallback = std::function<void(float)>;
    using Vec2Callback = std::function<void(float, float)>;
    using Vec3Callback = std::function<void(float, float, float)>;
    using TriggerCallback = std::function<void()>;

    struct Mapping {
        std::string id;
        std::string name;
        MidiTriggerType triggerType;
        uint8_t channel;
        uint8_t noteOrCC;
        MappingMode mode;
        ParameterType paramType;
        ParameterRange range;

        ParameterCallback floatCallback;
        Vec2Callback vec2Callback;
        Vec3Callback vec3Callback;
        TriggerCallback triggerCallback;

        bool enabled = true;
        float currentValue = 0.0f;
        bool noteHeld = false;

        Mapping()
            : triggerType(MidiTriggerType::NoteOn)
            , channel(0)
            , noteOrCC(0)
            , mode(MappingMode::Direct)
            , paramType(ParameterType::Float)
        {}
    };

    MidiMapping();

    void ProcessMidiEvent(const MidiEvent& event);

    std::string AddFloatMapping(
        const std::string& name,
        MidiTriggerType triggerType,
        uint8_t channel,
        uint8_t noteOrCC,
        ParameterCallback callback,
        MappingMode mode = MappingMode::Direct,
        const ParameterRange& range = ParameterRange()
    );

    std::string AddVec3Mapping(
        const std::string& name,
        MidiTriggerType triggerType,
        uint8_t channel,
        uint8_t noteOrCC,
        Vec3Callback callback,
        int componentIndex,
        MappingMode mode = MappingMode::Direct,
        const ParameterRange& range = ParameterRange()
    );

    std::string AddTriggerMapping(
        const std::string& name,
        MidiTriggerType triggerType,
        uint8_t channel,
        uint8_t noteOrCC,
        TriggerCallback callback
    );

    void RemoveMapping(const std::string& id);
    void ClearMappings();

    Mapping* GetMapping(const std::string& id);
    std::vector<Mapping*> GetAllMappings();

    void EnableMapping(const std::string& id, bool enabled);
    bool IsMappingEnabled(const std::string& id) const;

    nlohmann::json SerializeToJson() const;
    void DeserializeFromJson(const nlohmann::json& json);

    bool SaveToFile(const std::string& filename) const;
    bool LoadFromFile(const std::string& filename);

private:
    std::unordered_map<std::string, Mapping> mappings;
    int nextMappingId = 0;

    std::string GenerateMappingId();
    void ProcessMapping(Mapping& mapping, const MidiEvent& event);
};

} // namespace mo3d
