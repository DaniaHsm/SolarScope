#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

// Manages the space background environment:
// - Creates a skybox cube with 6 textured faces
// - Handles cubemap texture loading
// - Renders the environment around the scene
class Skybox {
public:
    GLuint vao;     // Vertex Array Object for the cube
    GLuint vbo;     // Vertex Buffer Object
    GLuint texture; // Texture ID

    // Factory method to create a skybox
    static Skybox create(const std::vector<std::string>& faces);

    // Renders the skybox using the provided shader and matrices
    void render(GLuint shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) const;

private:
    // Helper method to load cubemap textures
    static unsigned int loadCubemap(const std::vector<std::string>& faces);
};
