#include "SphereGeometry.hpp"
#include <glm/gtc/constants.hpp>

void SphereGeometry::generateSphereVerticesAndUVs(unsigned int rings,
                                                 unsigned int sectors,
                                                 std::vector<glm::vec3>& vertices,
                                                 std::vector<glm::vec2>& uvs) {
    float const R = 1.0f / float(rings - 1);   // Ring step
    float const S = 1.0f / float(sectors - 1); // Sector step

    for (unsigned int r = 0; r < rings; ++r) {
        for (unsigned int s = 0; s < sectors; ++s) {
            float const phi = -glm::half_pi<float>() + glm::pi<float>() * r * R;
            float const theta = 2 * glm::pi<float>() * s * S;

            float const y = sin(phi);
            float const x = cos(theta) * sin(glm::pi<float>() * r * R);
            float const z = sin(theta) * sin(glm::pi<float>() * r * R);

            vertices.push_back(glm::vec3(x, y, z));
            uvs.push_back(glm::vec2(s * S, r * R));
        }
    }
}

void SphereGeometry::generateSphereIndices(unsigned int rings,
                                         unsigned int sectors,
                                         std::vector<unsigned int>& indices) {
    for (unsigned int r = 0; r < rings - 1; ++r) {
        for (unsigned int s = 0; s < sectors - 1; ++s) {
            unsigned int current = r * sectors + s;
            unsigned int next = current + 1;
            unsigned int below = (r + 1) * sectors + s;
            unsigned int belowNext = below + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(belowNext);

            indices.push_back(current);
            indices.push_back(belowNext);
            indices.push_back(below);
        }
    }
}

GLuint SphereGeometry::setupSphereBuffers(const std::vector<glm::vec3>& vertices,
                                        const std::vector<glm::vec2>& uvs,
                                        const std::vector<unsigned int>& indices) {
    GLuint vao, vbo[2], ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(2, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    return vao;
}

GLuint SphereGeometry::createTexturedSphereVAO(unsigned int rings,
                                              unsigned int sectors,
                                              unsigned int& indexCount) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;

    generateSphereVerticesAndUVs(rings, sectors, vertices, uvs);
    generateSphereIndices(rings, sectors, indices);

    indexCount = indices.size();
    return setupSphereBuffers(vertices, uvs, indices);
}
