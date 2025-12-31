#include "Scene.h"
#include "../utils/Logger.h"

namespace mo3d {

Scene::Scene(const std::string& name)
    : name(name)
{
    mainCamera = std::make_shared<Camera>();
    mainCamera->transform.SetPosition(0.0f, 0.0f, 5.0f);
    LOG_INFO("Created scene: ", name);
}

std::shared_ptr<SceneObject> Scene::CreateObject(const std::string& name) {
    auto object = std::make_shared<SceneObject>(name);
    AddObject(object);
    return object;
}

void Scene::AddObject(std::shared_ptr<SceneObject> object) {
    objects.push_back(object);
    objectsByName[object->GetName()] = object;
}

void Scene::RemoveObject(std::shared_ptr<SceneObject> object) {
    auto it = std::find(objects.begin(), objects.end(), object);
    if (it != objects.end()) {
        objectsByName.erase(object->GetName());
        objects.erase(it);
    }
}

void Scene::RemoveObject(const std::string& name) {
    auto object = FindObject(name);
    if (object) {
        RemoveObject(object);
    }
}

void Scene::Clear() {
    objects.clear();
    objectsByName.clear();
}

std::shared_ptr<SceneObject> Scene::FindObject(const std::string& name) const {
    auto it = objectsByName.find(name);
    return it != objectsByName.end() ? it->second : nullptr;
}

std::vector<std::shared_ptr<SceneObject>> Scene::GetAllObjects() const {
    return objects;
}

void Scene::Update(float deltaTime) {
    for (auto& object : objects) {
        if (object && object->IsEnabled()) {
            object->Update(deltaTime);
        }
    }
}

} // namespace mo3d
