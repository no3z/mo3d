#include "Mesh.h"
#include "../utils/Logger.h"

#ifdef __APPLE__
    #include <OpenGL/gl3.h>
#else
    #include <GL/gl.h>
#endif

#include <glm/gtc/constants.hpp>

namespace mo3d {

Mesh::Mesh()
    : VAO(0), VBO(0), EBO(0), uploaded(false)
{
}

Mesh::~Mesh() {
    Cleanup();
}

void Mesh::SetVertices(const std::vector<Vertex>& verts) {
    vertices = verts;
    uploaded = false;
}

void Mesh::SetIndices(const std::vector<unsigned int>& inds) {
    indices = inds;
    uploaded = false;
}

void Mesh::Upload() {
    if (vertices.empty()) return;

    Cleanup();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    if (!indices.empty()) {
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);

    uploaded = true;
}

void Mesh::Draw() const {
    if (!uploaded || VAO == 0) return;

    glBindVertexArray(VAO);

    if (EBO != 0) {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    }

    glBindVertexArray(0);
}

void Mesh::Cleanup() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (EBO != 0) {
        glDeleteBuffers(1, &EBO);
        EBO = 0;
    }
    uploaded = false;
}

Mesh* Mesh::CreateCube() {
    Mesh* mesh = new Mesh();

    std::vector<Vertex> vertices = {
        Vertex(glm::vec3(-0.5f, -0.5f, -0.5f)),
        Vertex(glm::vec3( 0.5f, -0.5f, -0.5f)),
        Vertex(glm::vec3( 0.5f,  0.5f, -0.5f)),
        Vertex(glm::vec3(-0.5f,  0.5f, -0.5f)),
        Vertex(glm::vec3(-0.5f, -0.5f,  0.5f)),
        Vertex(glm::vec3( 0.5f, -0.5f,  0.5f)),
        Vertex(glm::vec3( 0.5f,  0.5f,  0.5f)),
        Vertex(glm::vec3(-0.5f,  0.5f,  0.5f))
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,
        1, 5, 6, 6, 2, 1,
        5, 4, 7, 7, 6, 5,
        4, 0, 3, 3, 7, 4,
        3, 2, 6, 6, 7, 3,
        4, 5, 1, 1, 0, 4
    };

    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);
    mesh->Upload();

    return mesh;
}

Mesh* Mesh::CreateSphere(int segments) {
    Mesh* mesh = new Mesh();
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * glm::pi<float>() / segments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2 * glm::pi<float>() / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            Vertex v;
            v.position = glm::vec3(cosPhi * sinTheta, cosTheta, sinPhi * sinTheta) * 0.5f;
            v.normal = glm::normalize(v.position);
            v.texCoord = glm::vec2((float)lon / segments, (float)lat / segments);

            vertices.push_back(v);
        }
    }

    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = lat * (segments + 1) + lon;
            int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);
    mesh->Upload();

    return mesh;
}

Mesh* Mesh::CreatePlane() {
    Mesh* mesh = new Mesh();

    std::vector<Vertex> vertices = {
        Vertex(glm::vec3(-0.5f, 0.0f, -0.5f)),
        Vertex(glm::vec3( 0.5f, 0.0f, -0.5f)),
        Vertex(glm::vec3( 0.5f, 0.0f,  0.5f)),
        Vertex(glm::vec3(-0.5f, 0.0f,  0.5f))
    };

    std::vector<unsigned int> indices = { 0, 1, 2, 2, 3, 0 };

    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);
    mesh->Upload();

    return mesh;
}

Mesh* Mesh::CreateQuad() {
    Mesh* mesh = new Mesh();

    std::vector<Vertex> vertices = {
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f))
    };

    vertices[0].texCoord = glm::vec2(0.0f, 0.0f);
    vertices[1].texCoord = glm::vec2(1.0f, 0.0f);
    vertices[2].texCoord = glm::vec2(1.0f, 1.0f);
    vertices[3].texCoord = glm::vec2(0.0f, 1.0f);

    std::vector<unsigned int> indices = { 0, 1, 2, 2, 3, 0 };

    mesh->SetVertices(vertices);
    mesh->SetIndices(indices);
    mesh->Upload();

    return mesh;
}

} // namespace mo3d
