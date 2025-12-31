#pragma once

#include <string>

namespace mo3d {

class Texture {
public:
    Texture();
    ~Texture();

    bool LoadFromFile(const std::string& path);
    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    unsigned int GetID() const { return textureID; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    unsigned int textureID;
    int width;
    int height;
    int channels;
};

} // namespace mo3d
