#include "FileDialog.h"
#include "../utils/Logger.h"
#include <imgui.h>
#include <filesystem>
#include <algorithm>

namespace mo3d {

namespace fs = std::filesystem;

FileDialog::FileDialog()
    : isOpen(false)
    , mode(FileDialogMode::Open)
    , currentPath(".")
    , fileExtension(".mo3d")
{
    inputFileName.resize(256);
}

void FileDialog::Open(FileDialogMode dialogMode, const std::string& dialogTitle, const std::string& defaultPath) {
    isOpen = true;
    mode = dialogMode;
    title = dialogTitle;
    currentPath = defaultPath;
    selectedPath.clear();

    if (mode == FileDialogMode::Save) {
        strcpy(inputFileName.data(), "untitled");
    }

    RefreshDirectory();
}

void FileDialog::Close() {
    isOpen = false;
    selectedPath.clear();
}

bool FileDialog::Render() {
    if (!isOpen) return false;

    bool fileSelected = false;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(title.c_str(), &isOpen, ImGuiWindowFlags_NoCollapse)) {

        // Current path display
        ImGui::Text("Path: %s", currentPath.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Up")) {
            NavigateUp();
        }
        ImGui::SameLine();
        if (ImGui::Button("Refresh")) {
            RefreshDirectory();
        }

        ImGui::Separator();

        // File list
        ImGui::BeginChild("FileList", ImVec2(0, -60), true);

        // Directories
        for (const auto& dir : directories) {
            ImGui::PushID(dir.c_str());
            if (ImGui::Selectable(("📁 " + dir).c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    NavigateTo(currentPath + "/" + dir);
                }
            }
            ImGui::PopID();
        }

        // Files
        for (const auto& file : files) {
            ImGui::PushID(file.c_str());
            bool selected = (selectedPath == (currentPath + "/" + file));

            if (ImGui::Selectable(("📄 " + file).c_str(), selected, ImGuiSelectableFlags_AllowDoubleClick)) {
                selectedPath = currentPath + "/" + file;

                if (mode == FileDialogMode::Save) {
                    strcpy(inputFileName.data(), file.c_str());
                }

                if (ImGui::IsMouseDoubleClicked(0) && mode == FileDialogMode::Open) {
                    fileSelected = true;
                    isOpen = false;
                }
            }
            ImGui::PopID();
        }

        ImGui::EndChild();

        ImGui::Separator();

        // Input/output area
        if (mode == FileDialogMode::Save) {
            ImGui::Text("File name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-100);
            ImGui::InputText("##filename", inputFileName.data(), 256);
        } else {
            ImGui::Text("Selected: %s", selectedPath.empty() ? "None" : selectedPath.c_str());
        }

        // Buttons
        ImGui::Separator();

        if (mode == FileDialogMode::Open) {
            if (ImGui::Button("Open", ImVec2(100, 0))) {
                if (!selectedPath.empty()) {
                    fileSelected = true;
                    isOpen = false;
                }
            }
        } else {
            if (ImGui::Button("Save", ImVec2(100, 0))) {
                std::string filename = inputFileName.data();
                if (!filename.empty()) {
                    selectedPath = currentPath + "/" + filename;

                    // Add extension if not present
                    if (!fileExtension.empty() &&
                        selectedPath.find(fileExtension) == std::string::npos) {
                        selectedPath += fileExtension;
                    }

                    fileSelected = true;
                    isOpen = false;
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0))) {
            Close();
        }
    }
    ImGui::End();

    return fileSelected;
}

void FileDialog::RefreshDirectory() {
    directories.clear();
    files.clear();

    try {
        for (const auto& entry : fs::directory_iterator(currentPath)) {
            if (entry.is_directory()) {
                directories.push_back(entry.path().filename().string());
            } else if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();

                // Filter by extension if set
                if (fileExtension.empty() || filename.find(fileExtension) != std::string::npos) {
                    files.push_back(filename);
                }
            }
        }

        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());

    } catch (const std::exception& e) {
        LOG_ERROR("Failed to read directory: ", e.what());
    }
}

void FileDialog::NavigateUp() {
    try {
        fs::path path(currentPath);
        if (path.has_parent_path()) {
            currentPath = path.parent_path().string();
            RefreshDirectory();
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to navigate up: ", e.what());
    }
}

void FileDialog::NavigateTo(const std::string& path) {
    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            currentPath = path;
            RefreshDirectory();
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to navigate to: ", path, " - ", e.what());
    }
}

} // namespace mo3d
