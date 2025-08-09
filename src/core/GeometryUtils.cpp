#include "GeometryUtils.hpp"

namespace GeometryUtils {

GLuint createVertexBufferObject() {
    glm::vec3 vertexArray[] = {
        // Front face (red)
        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f),

        // Back face (blue)
        glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f),

        glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f),

        // Bottom face (cyan)
        glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 1.0f),
        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 1.0f),
        glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 1.0f),

        glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 1.0f),
        glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 1.0f),
        glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 1.0f),

        // Top face (green)
        glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),

        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f),

        // Right face (magenta)
        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f),

        glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 1.0f),

        // Left face (yellow)
        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 0.0f),

        glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 0.0f)
    };

    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    return vertexBufferObject;
}

}
