#pragma once

#include "SceneObject.h"
#include "Camera.h"
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace mo3d {

class Scene {
public:
    Scene(const std::string& name = "MainScene");
    ~Scene() = default;

    std::string GetName() const { return name; }
    void SetName(const std::string& n) { name = n; }

    std::shared_ptr<SceneObject> CreateObject(const std::string& name = "Object");
    void AddObject(std::shared_ptr<SceneObject> object);
    void RemoveObject(std::shared_ptr<SceneObject> object);
    void RemoveObject(const std::string& name);
    void Clear();

    std::shared_ptr<SceneObject> FindObject(const std::string& name) const;
    std::vector<std::shared_ptr<SceneObject>> GetAllObjects() const;

    void SetMainCamera(std::shared_ptr<Camera> cam) { mainCamera = cam; }
    std::shared_ptr<Camera> GetMainCamera() const { return mainCamera; }

    void Update(float deltaTime);

    size_t GetObjectCount() const { return objects.size(); }

private:
    std::string name;
    std::vector<std::shared_ptr<SceneObject>> objects;
    std::unordered_map<std::string, std::shared_ptr<SceneObject>> objectsByName;
    std::shared_ptr<Camera> mainCamera;
};

} // namespace mo3d
