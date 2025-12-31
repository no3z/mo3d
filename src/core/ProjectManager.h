#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace mo3d {

class Scene;
class MidiMapping;

struct ProjectInfo {
    std::string name = "Untitled Project";
    std::string author;
    std::string description;
    std::string version = "1.0";
    std::string createdDate;
    std::string modifiedDate;
};

class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager() = default;

    bool NewProject(const std::string& name = "Untitled");
    bool SaveProject(const std::string& filepath);
    bool LoadProject(const std::string& filepath);

    bool SaveProjectAs(const std::string& filepath);
    bool HasUnsavedChanges() const { return unsavedChanges; }
    void MarkSaved() { unsavedChanges = false; }
    void MarkDirty() { unsavedChanges = true; }

    const std::string& GetCurrentProjectPath() const { return currentProjectPath; }
    const ProjectInfo& GetProjectInfo() const { return projectInfo; }
    void SetProjectInfo(const ProjectInfo& info) { projectInfo = info; MarkDirty(); }

    nlohmann::json SerializeProject(Scene* scene, MidiMapping* mapping) const;
    bool DeserializeProject(const nlohmann::json& json, Scene* scene, MidiMapping* mapping);

    std::string GetRecentProjectsPath() const { return "recent_projects.json"; }
    void AddToRecentProjects(const std::string& filepath);
    std::vector<std::string> GetRecentProjects() const;

private:
    ProjectInfo projectInfo;
    std::string currentProjectPath;
    bool unsavedChanges;

    std::string GetCurrentDateTime() const;
};

} // namespace mo3d
