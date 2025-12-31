#pragma once

#include <string>
#include <vector>
#include <functional>

namespace mo3d {

enum class FileDialogMode {
    Open,
    Save
};

class FileDialog {
public:
    FileDialog();

    void Open(FileDialogMode mode, const std::string& title = "Select File", const std::string& defaultPath = ".");
    void Close();

    bool Render();  // Returns true if file was selected
    bool IsOpen() const { return isOpen; }

    std::string GetSelectedPath() const { return selectedPath; }
    void SetFileExtension(const std::string& ext) { fileExtension = ext; }

private:
    bool isOpen;
    FileDialogMode mode;
    std::string title;
    std::string currentPath;
    std::string selectedPath;
    std::string fileExtension;
    std::string inputFileName;

    std::vector<std::string> directories;
    std::vector<std::string> files;

    void RefreshDirectory();
    void NavigateUp();
    void NavigateTo(const std::string& path);
};

} // namespace mo3d
