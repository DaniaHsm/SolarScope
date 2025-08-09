#include "CometFactory.hpp"
#include "CelestialBodyFactory.hpp"
#include <GLFW/glfw3.h>

Comet CometFactory::createComet(const char* texturePath, 
                               const glm::vec3& orbitCenter, 
                               float semiMajorAxis, 
                               float eccentricity) {
    Comet comet;

    // Create the comet head using existing celestial body system
    comet.body = CelestialBodyFactory::createCelestialBody(texturePath, 0.05f, 0.0f, 0.0f, 10.0f);

    // Set up orbital parameters
    comet.orbitCenter = orbitCenter;
    comet.semiMajorAxis = semiMajorAxis;
    comet.eccentricity = eccentricity;
    comet.orbitAngle = 0.0f;
    comet.maxTrailPoints = 150; // Long, visible trail
    comet.lastTrailUpdate = 0.0f;

    // Set up trail rendering
    glGenVertexArrays(1, &comet.trailVAO);
    glGenBuffers(1, &comet.trailVBO);

    return comet;
}

void CometFactory::updateComet(Comet& comet, float dt, const glm::vec3& sunPosition) {
    // Update orbital position
    comet.orbitAngle += 0.5f * dt; // Slow orbital speed

    // Calculate elliptical orbit position
    float a = comet.semiMajorAxis; // Semi-major axis
    float e = comet.eccentricity;  // Eccentricity

    // Elliptical orbit math
    float r = a * (1.0f - e * e) / (1.0f + e * cos(comet.orbitAngle));
    float x = r * cos(comet.orbitAngle);
    float z = r * sin(comet.orbitAngle);

    comet.body.position = comet.orbitCenter + glm::vec3(x, 0.0f, z);

    // Update rotation
    comet.body.rotationAngle += comet.body.rotationSpeed * dt;

    // Update trail
    comet.updateTrail(glfwGetTime(), sunPosition);
}

void CometFactory::renderCometTrail(const Comet& comet, 
                                  GLuint shader, 
                                  const glm::mat4& viewMatrix, 
                                  const glm::mat4& projectionMatrix) {
    if (comet.trail.size() < 2)
        return;

    glUseProgram(shader);
    glBindVertexArray(comet.trailVAO);

    // Set matrices
    glm::mat4 worldMatrix(1.0f); // Identity - trail points are in world space
    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);

    // Enable blending for trail transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw trail as line strip
    glDrawArrays(GL_LINE_STRIP, 0, comet.trail.size());

    glDisable(GL_BLEND);
}
