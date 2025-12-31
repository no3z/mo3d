#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>

namespace mo3d {

class FileIO {
public:
    static std::string ReadTextFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static std::vector<char> ReadBinaryFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return {};
        }
        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        return buffer;
    }

    static bool WriteTextFile(const std::string& path, const std::string& content) {
        std::ofstream file(path);
        if (!file.is_open()) {
            return false;
        }
        file << content;
        return true;
    }

    static bool FileExists(const std::string& path) {
        std::ifstream file(path);
        return file.good();
    }
};

} // namespace mo3d
