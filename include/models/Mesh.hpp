#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

struct Mesh {
    std::vector<glm::vec3> vertices;    // 3D vertex positions
    std::vector<glm::vec3> normals;     // Vertex normals for lighting
    std::vector<glm::vec2> texCoords;   // Texture coordinates
    std::vector<unsigned int> indices;   // Vertex indices for drawing
    GLuint VAO;                         // Vertex Array Object
    GLuint texture;                     // Diffuse texture

    void setupMesh();
};
