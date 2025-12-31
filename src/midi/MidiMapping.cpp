#include "MidiMapping.h"
#include "../utils/Logger.h"
#include "../utils/FileIO.h"
#include <sstream>

namespace mo3d {

MidiMapping::MidiMapping() {
}

void MidiMapping::ProcessMidiEvent(const MidiEvent& event) {
    for (auto& [id, mapping] : mappings) {
        if (!mapping.enabled) continue;

        bool shouldProcess = false;

        switch (mapping.triggerType) {
            case MidiTriggerType::NoteOn:
                shouldProcess = event.IsNoteOn() &&
                               event.channel == mapping.channel &&
                               event.GetNote() == mapping.noteOrCC;
                break;

            case MidiTriggerType::NoteOff:
                shouldProcess = event.IsNoteOff() &&
                               event.channel == mapping.channel &&
                               event.GetNote() == mapping.noteOrCC;
                break;

            case MidiTriggerType::NoteAny:
                shouldProcess = (event.IsNoteOn() || event.IsNoteOff()) &&
                               event.channel == mapping.channel &&
                               event.GetNote() == mapping.noteOrCC;
                break;

            case MidiTriggerType::CC:
            case MidiTriggerType::CCValue:
                shouldProcess = event.IsCC() &&
                               event.channel == mapping.channel &&
                               event.GetCCNumber() == mapping.noteOrCC;
                break;

            case MidiTriggerType::Velocity:
                shouldProcess = event.IsNoteOn() &&
                               event.channel == mapping.channel &&
                               event.GetNote() == mapping.noteOrCC;
                break;
        }

        if (shouldProcess) {
            ProcessMapping(mapping, event);
        }
    }
}

void MidiMapping::ProcessMapping(Mapping& mapping, const MidiEvent& event) {
    float inputValue = 0.0f;

    switch (mapping.triggerType) {
        case MidiTriggerType::Velocity:
            inputValue = event.GetNormalizedVelocity() * 127.0f;
            break;
        case MidiTriggerType::CCValue:
            inputValue = static_cast<float>(event.GetCCValue());
            break;
        default:
            inputValue = event.data2;
            break;
    }

    switch (mapping.mode) {
        case MappingMode::Direct: {
            float normalizedValue = mapping.range.Normalize(inputValue);
            mapping.currentValue = normalizedValue;

            if (mapping.floatCallback) {
                mapping.floatCallback(normalizedValue);
            }
            break;
        }

        case MappingMode::Toggle: {
            if (event.IsNoteOn() || (event.IsCC() && event.GetCCValue() > 0)) {
                mapping.currentValue = mapping.currentValue > 0.5f ? 0.0f : 1.0f;
                if (mapping.floatCallback) {
                    mapping.floatCallback(mapping.currentValue);
                }
            }
            break;
        }

        case MappingMode::Trigger: {
            if (event.IsNoteOn() || event.IsCC()) {
                if (mapping.triggerCallback) {
                    mapping.triggerCallback();
                }
            }
            break;
        }

        case MappingMode::Momentary: {
            if (event.IsNoteOn()) {
                mapping.noteHeld = true;
                mapping.currentValue = mapping.range.max;
            } else if (event.IsNoteOff()) {
                mapping.noteHeld = false;
                mapping.currentValue = mapping.range.min;
            }

            if (mapping.floatCallback) {
                mapping.floatCallback(mapping.currentValue);
            }
            break;
        }

        case MappingMode::Increment: {
            if (event.IsNoteOn() || (event.IsCC() && event.GetCCValue() > 0)) {
                float step = (mapping.range.max - mapping.range.min) * 0.1f;
                mapping.currentValue = mapping.range.Clamp(mapping.currentValue + step);
                if (mapping.floatCallback) {
                    mapping.floatCallback(mapping.currentValue);
                }
            }
            break;
        }

        case MappingMode::Decrement: {
            if (event.IsNoteOn() || (event.IsCC() && event.GetCCValue() > 0)) {
                float step = (mapping.range.max - mapping.range.min) * 0.1f;
                mapping.currentValue = mapping.range.Clamp(mapping.currentValue - step);
                if (mapping.floatCallback) {
                    mapping.floatCallback(mapping.currentValue);
                }
            }
            break;
        }
    }
}

std::string MidiMapping::AddFloatMapping(
    const std::string& name,
    MidiTriggerType triggerType,
    uint8_t channel,
    uint8_t noteOrCC,
    ParameterCallback callback,
    MappingMode mode,
    const ParameterRange& range)
{
    std::string id = GenerateMappingId();

    Mapping mapping;
    mapping.id = id;
    mapping.name = name;
    mapping.triggerType = triggerType;
    mapping.channel = channel;
    mapping.noteOrCC = noteOrCC;
    mapping.mode = mode;
    mapping.paramType = ParameterType::Float;
    mapping.range = range;
    mapping.floatCallback = callback;
    mapping.currentValue = range.defaultValue;

    mappings[id] = mapping;

    LOG_INFO("Added float mapping: ", name, " [Ch:", (int)channel, " Note/CC:", (int)noteOrCC, "]");
    return id;
}

std::string MidiMapping::AddVec3Mapping(
    const std::string& name,
    MidiTriggerType triggerType,
    uint8_t channel,
    uint8_t noteOrCC,
    Vec3Callback callback,
    int componentIndex,
    MappingMode mode,
    const ParameterRange& range)
{
    std::string id = GenerateMappingId();

    Mapping mapping;
    mapping.id = id;
    mapping.name = name;
    mapping.triggerType = triggerType;
    mapping.channel = channel;
    mapping.noteOrCC = noteOrCC;
    mapping.mode = mode;
    mapping.paramType = ParameterType::Vec3;
    mapping.range = range;
    mapping.vec3Callback = callback;
    mapping.currentValue = range.defaultValue;

    mappings[id] = mapping;

    LOG_INFO("Added Vec3 mapping: ", name);
    return id;
}

std::string MidiMapping::AddTriggerMapping(
    const std::string& name,
    MidiTriggerType triggerType,
    uint8_t channel,
    uint8_t noteOrCC,
    TriggerCallback callback)
{
    std::string id = GenerateMappingId();

    Mapping mapping;
    mapping.id = id;
    mapping.name = name;
    mapping.triggerType = triggerType;
    mapping.channel = channel;
    mapping.noteOrCC = noteOrCC;
    mapping.mode = MappingMode::Trigger;
    mapping.triggerCallback = callback;

    mappings[id] = mapping;

    LOG_INFO("Added trigger mapping: ", name);
    return id;
}

void MidiMapping::RemoveMapping(const std::string& id) {
    mappings.erase(id);
}

void MidiMapping::ClearMappings() {
    mappings.clear();
}

MidiMapping::Mapping* MidiMapping::GetMapping(const std::string& id) {
    auto it = mappings.find(id);
    return it != mappings.end() ? &it->second : nullptr;
}

std::vector<MidiMapping::Mapping*> MidiMapping::GetAllMappings() {
    std::vector<Mapping*> result;
    for (auto& [id, mapping] : mappings) {
        result.push_back(&mapping);
    }
    return result;
}

void MidiMapping::EnableMapping(const std::string& id, bool enabled) {
    auto it = mappings.find(id);
    if (it != mappings.end()) {
        it->second.enabled = enabled;
    }
}

bool MidiMapping::IsMappingEnabled(const std::string& id) const {
    auto it = mappings.find(id);
    return it != mappings.end() ? it->second.enabled : false;
}

std::string MidiMapping::GenerateMappingId() {
    std::ostringstream oss;
    oss << "mapping_" << nextMappingId++;
    return oss.str();
}

nlohmann::json MidiMapping::SerializeToJson() const {
    nlohmann::json json = nlohmann::json::array();

    for (const auto& [id, mapping] : mappings) {
        nlohmann::json mappingJson;
        mappingJson["id"] = mapping.id;
        mappingJson["name"] = mapping.name;
        mappingJson["triggerType"] = static_cast<int>(mapping.triggerType);
        mappingJson["channel"] = mapping.channel;
        mappingJson["noteOrCC"] = mapping.noteOrCC;
        mappingJson["mode"] = static_cast<int>(mapping.mode);
        mappingJson["paramType"] = static_cast<int>(mapping.paramType);
        mappingJson["range"]["min"] = mapping.range.min;
        mappingJson["range"]["max"] = mapping.range.max;
        mappingJson["range"]["default"] = mapping.range.defaultValue;
        mappingJson["enabled"] = mapping.enabled;

        json.push_back(mappingJson);
    }

    return json;
}

void MidiMapping::DeserializeFromJson(const nlohmann::json& json) {
    // Note: This doesn't restore callbacks, which must be re-registered
    // after loading
    LOG_WARN("Loading mappings from JSON - callbacks need to be re-registered");
}

bool MidiMapping::SaveToFile(const std::string& filename) const {
    try {
        nlohmann::json json = SerializeToJson();
        std::string content = json.dump(2);
        return FileIO::WriteTextFile(filename, content);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save mappings: ", e.what());
        return false;
    }
}

bool MidiMapping::LoadFromFile(const std::string& filename) {
    try {
        std::string content = FileIO::ReadTextFile(filename);
        if (content.empty()) {
            LOG_ERROR("Failed to read mapping file");
            return false;
        }

        nlohmann::json json = nlohmann::json::parse(content);
        DeserializeFromJson(json);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load mappings: ", e.what());
        return false;
    }
}

} // namespace mo3d
