#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace mo3d {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec4 color;

    Vertex()
        : position(0.0f)
        , normal(0.0f, 1.0f, 0.0f)
        , texCoord(0.0f)
        , color(1.0f)
    {}

    Vertex(const glm::vec3& pos)
        : position(pos)
        , normal(0.0f, 1.0f, 0.0f)
        , texCoord(0.0f)
        , color(1.0f)
    {}
};

class Mesh {
public:
    Mesh();
    ~Mesh();

    void SetVertices(const std::vector<Vertex>& verts);
    void SetIndices(const std::vector<unsigned int>& inds);

    void Upload();
    void Draw() const;

    static Mesh* CreateCube();
    static Mesh* CreateSphere(int segments = 32);
    static Mesh* CreatePlane();
    static Mesh* CreateQuad();

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int VAO, VBO, EBO;
    bool uploaded;

    void SetupMesh();
    void Cleanup();
};

} // namespace mo3d
