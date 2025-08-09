#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "CelestialBody.hpp"

struct PlanetRing {
    GLuint vao;
    GLuint texture;
    unsigned int indexCount;
    float innerRadius;
    float outerRadius;

    // Render the planet ring
    void render(const CelestialBody& planet,
                GLuint shader,
                const glm::mat4& viewMatrix,
                const glm::mat4& projectionMatrix,
                const glm::vec3& lightPos,
                const glm::vec3& viewPos) const;
};
