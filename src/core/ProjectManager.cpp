#include "ProjectManager.h"
#include "../scene/Scene.h"
#include "../scene/SceneObject.h"
#include "../midi/MidiMapping.h"
#include "../utils/Logger.h"
#include "../utils/FileIO.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace mo3d {

ProjectManager::ProjectManager()
    : unsavedChanges(false)
{
}

bool ProjectManager::NewProject(const std::string& name) {
    projectInfo = ProjectInfo();
    projectInfo.name = name;
    projectInfo.createdDate = GetCurrentDateTime();
    projectInfo.modifiedDate = projectInfo.createdDate;

    currentProjectPath.clear();
    unsavedChanges = false;

    LOG_INFO("Created new project: ", name);
    return true;
}

bool ProjectManager::SaveProject(const std::string& filepath) {
    // This will be called with scene and mapping from Application
    currentProjectPath = filepath;
    projectInfo.modifiedDate = GetCurrentDateTime();

    AddToRecentProjects(filepath);

    LOG_INFO("Project saved to: ", filepath);
    return true;
}

bool ProjectManager::LoadProject(const std::string& filepath) {
    try {
        std::string content = FileIO::ReadTextFile(filepath);
        if (content.empty()) {
            LOG_ERROR("Failed to read project file: ", filepath);
            return false;
        }

        nlohmann::json json = nlohmann::json::parse(content);

        currentProjectPath = filepath;
        unsavedChanges = false;

        // Extract project info
        if (json.contains("projectInfo")) {
            auto& info = json["projectInfo"];
            if (info.contains("name")) projectInfo.name = info["name"];
            if (info.contains("author")) projectInfo.author = info["author"];
            if (info.contains("description")) projectInfo.description = info["description"];
            if (info.contains("version")) projectInfo.version = info["version"];
            if (info.contains("createdDate")) projectInfo.createdDate = info["createdDate"];
            if (info.contains("modifiedDate")) projectInfo.modifiedDate = info["modifiedDate"];
        }

        AddToRecentProjects(filepath);

        LOG_INFO("Project loaded from: ", filepath);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load project: ", e.what());
        return false;
    }
}

bool ProjectManager::SaveProjectAs(const std::string& filepath) {
    return SaveProject(filepath);
}

nlohmann::json ProjectManager::SerializeProject(Scene* scene, MidiMapping* mapping) const {
    nlohmann::json json;

    // Project info
    json["projectInfo"]["name"] = projectInfo.name;
    json["projectInfo"]["author"] = projectInfo.author;
    json["projectInfo"]["description"] = projectInfo.description;
    json["projectInfo"]["version"] = projectInfo.version;
    json["projectInfo"]["createdDate"] = projectInfo.createdDate;
    json["projectInfo"]["modifiedDate"] = GetCurrentDateTime();

    // Scene data
    if (scene) {
        nlohmann::json sceneJson;
        sceneJson["name"] = scene->GetName();

        // Camera
        auto camera = scene->GetMainCamera();
        if (camera) {
            auto pos = camera->transform.GetPosition();
            auto rot = camera->transform.GetRotationEuler();

            sceneJson["camera"]["position"] = {pos.x, pos.y, pos.z};
            sceneJson["camera"]["rotation"] = {rot.x, rot.y, rot.z};
            sceneJson["camera"]["fov"] = camera->GetFOV();
            sceneJson["camera"]["nearPlane"] = camera->GetNearPlane();
            sceneJson["camera"]["farPlane"] = camera->GetFarPlane();
        }

        // Objects
        auto objects = scene->GetAllObjects();
        nlohmann::json objectsJson = nlohmann::json::array();

        for (const auto& obj : objects) {
            nlohmann::json objJson;
            objJson["name"] = obj->GetName();
            objJson["enabled"] = obj->IsEnabled();

            auto pos = obj->transform.GetPosition();
            auto rot = obj->transform.GetRotationEuler();
            auto scale = obj->transform.GetScale();

            objJson["transform"]["position"] = {pos.x, pos.y, pos.z};
            objJson["transform"]["rotation"] = {rot.x, rot.y, rot.z};
            objJson["transform"]["scale"] = {scale.x, scale.y, scale.z};

            auto color = obj->renderProps.color;
            objJson["renderProps"]["color"] = {color.r, color.g, color.b, color.a};
            objJson["renderProps"]["opacity"] = obj->renderProps.opacity;
            objJson["renderProps"]["visible"] = obj->renderProps.visible;

            // TODO: Mesh type, material info

            objectsJson.push_back(objJson);
        }

        sceneJson["objects"] = objectsJson;
        json["scene"] = sceneJson;
    }

    // MIDI mappings
    if (mapping) {
        json["midiMappings"] = mapping->SerializeToJson();
    }

    return json;
}

bool ProjectManager::DeserializeProject(const nlohmann::json& json, Scene* scene, MidiMapping* mapping) {
    try {
        // Scene data
        if (json.contains("scene") && scene) {
            auto& sceneJson = json["scene"];

            if (sceneJson.contains("name")) {
                scene->SetName(sceneJson["name"]);
            }

            // Camera
            if (sceneJson.contains("camera")) {
                auto& camJson = sceneJson["camera"];
                auto camera = scene->GetMainCamera();

                if (camera) {
                    if (camJson.contains("position")) {
                        auto pos = camJson["position"];
                        camera->transform.SetPosition(pos[0], pos[1], pos[2]);
                    }

                    if (camJson.contains("rotation")) {
                        auto rot = camJson["rotation"];
                        camera->transform.SetRotationEuler(rot[0], rot[1], rot[2]);
                    }

                    if (camJson.contains("fov")) {
                        camera->SetFOV(camJson["fov"]);
                    }
                }
            }

            // Objects
            if (sceneJson.contains("objects")) {
                scene->Clear();

                for (const auto& objJson : sceneJson["objects"]) {
                    std::string name = objJson.value("name", "Object");
                    auto obj = scene->CreateObject(name);

                    if (objJson.contains("enabled")) {
                        obj->SetEnabled(objJson["enabled"]);
                    }

                    if (objJson.contains("transform")) {
                        auto& trans = objJson["transform"];

                        if (trans.contains("position")) {
                            auto pos = trans["position"];
                            obj->transform.SetPosition(pos[0], pos[1], pos[2]);
                        }

                        if (trans.contains("rotation")) {
                            auto rot = trans["rotation"];
                            obj->transform.SetRotationEuler(rot[0], rot[1], rot[2]);
                        }

                        if (trans.contains("scale")) {
                            auto scale = trans["scale"];
                            obj->transform.SetScale(scale[0], scale[1], scale[2]);
                        }
                    }

                    if (objJson.contains("renderProps")) {
                        auto& props = objJson["renderProps"];

                        if (props.contains("color")) {
                            auto color = props["color"];
                            obj->renderProps.color = glm::vec4(color[0], color[1], color[2], color[3]);
                        }

                        if (props.contains("opacity")) {
                            obj->renderProps.opacity = props["opacity"];
                        }

                        if (props.contains("visible")) {
                            obj->renderProps.visible = props["visible"];
                        }
                    }

                    // TODO: Load mesh and material
                }
            }
        }

        // MIDI mappings (Note: callbacks need to be re-registered after loading)
        if (json.contains("midiMappings") && mapping) {
            // mapping->DeserializeFromJson(json["midiMappings"]);
            LOG_WARN("MIDI mappings loaded but callbacks need manual re-registration");
        }

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deserialize project: ", e.what());
        return false;
    }
}

void ProjectManager::AddToRecentProjects(const std::string& filepath) {
    try {
        std::vector<std::string> recent = GetRecentProjects();

        // Remove if already exists
        recent.erase(std::remove(recent.begin(), recent.end(), filepath), recent.end());

        // Add to front
        recent.insert(recent.begin(), filepath);

        // Keep only 10 most recent
        if (recent.size() > 10) {
            recent.resize(10);
        }

        // Save
        nlohmann::json json = recent;
        FileIO::WriteTextFile(GetRecentProjectsPath(), json.dump(2));

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update recent projects: ", e.what());
    }
}

std::vector<std::string> ProjectManager::GetRecentProjects() const {
    try {
        std::string content = FileIO::ReadTextFile(GetRecentProjectsPath());
        if (content.empty()) {
            return {};
        }

        nlohmann::json json = nlohmann::json::parse(content);
        return json.get<std::vector<std::string>>();

    } catch (const std::exception& e) {
        return {};
    }
}

std::string ProjectManager::GetCurrentDateTime() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace mo3d
