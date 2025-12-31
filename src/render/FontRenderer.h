#pragma once

#include <string>
#include <glm/glm.hpp>

namespace mo3d {

class FontRenderer {
public:
    bool Initialize();
    void RenderText(const std::string& text, float x, float y, float scale, const glm::vec4& color);
};

} // namespace mo3d
