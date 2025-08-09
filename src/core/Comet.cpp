#include "Comet.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

Comet Comet::create(const char* texturePath,
                    const glm::vec3& orbitCenter,
                    float semiMajorAxis,
                    float eccentricity) {
    Comet comet;

    // Create the comet head using CelestialBody factory method
    comet.body = CelestialBody::create(texturePath, 0.05f, 0.0f, 0.0f, 10.0f);

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

void Comet::update(float dt, const glm::vec3& sunPosition) {
    // Update orbital position
    orbitAngle += 0.5f * dt; // Slow orbital speed

    // Calculate elliptical orbit position
    float a = semiMajorAxis; // Semi-major axis
    float e = eccentricity;  // Eccentricity

    // Elliptical orbit math
    float r = a * (1.0f - e * e) / (1.0f + e * cos(orbitAngle));
    float x = r * cos(orbitAngle);
    float z = r * sin(orbitAngle);

    body.position = orbitCenter + glm::vec3(x, 0.0f, z);

    // Update rotation
    body.rotationAngle += body.rotationSpeed * dt;

    // Update trail
    updateTrail(glfwGetTime(), sunPosition);
}

void Comet::updateTrail(float currentTime, const glm::vec3& sunPosition) {
    // Add new trail point every 0.1 seconds
    if (currentTime - lastTrailUpdate > 0.1f) {
        TrailPoint newPoint;
        newPoint.position = body.position;
        newPoint.age = 0.0f;

        // Brightness based on distance from sun (closer = brighter trail)
        float distanceFromSun = glm::length(body.position - sunPosition);
        newPoint.brightness = 1.0f / (1.0f + distanceFromSun * 0.1f);

        trail.insert(trail.begin(), newPoint);

        // Remove old trail points
        if (trail.size() > maxTrailPoints) {
            trail.pop_back();
        }

        lastTrailUpdate = currentTime;
    }

    // Age all trail points
    for (auto& point : trail) {
        point.age += 0.016f; // Approximate 60fps
    }

    updateTrailVBO();
}

void Comet::renderTrail(GLuint shader, 
                         const glm::mat4& viewMatrix, 
                         const glm::mat4& projectionMatrix) const {
    if (trail.size() < 2)
        return;

    glUseProgram(shader);
    glBindVertexArray(trailVAO);

    // Set matrices
    glm::mat4 worldMatrix(1.0f); // Identity - trail points are in world space
    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);

    // Enable blending for trail transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw trail as line strip
    glDrawArrays(GL_LINE_STRIP, 0, trail.size());

    glDisable(GL_BLEND);
}

void Comet::updateTrailVBO() {
    if (trail.empty())
        return;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> colors;

    // Create line segments for the trail
    for (size_t i = 0; i < trail.size(); ++i) {
        vertices.push_back(trail[i].position);

        // Color fades from bright blue/white to dark blue based on age
        float fade = 1.0f - (trail[i].age / 10.0f); // Fade over 10 seconds
        fade = std::max(0.0f, fade);

        // Comet tail color - blue/white mix
        glm::vec3 color = glm::vec3(0.7f + 0.3f * trail[i].brightness, 
                                   0.8f + 0.2f * trail[i].brightness, 
                                   1.0f) * fade;
        colors.push_back(color);
    }

    glBindVertexArray(trailVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(glm::vec3) + colors.size() * sizeof(glm::vec3),
                 nullptr,
                 GL_DYNAMIC_DRAW);

    // Upload vertices
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), &vertices[0]);

    // Upload colors
    glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), 
                   colors.size() * sizeof(glm::vec3), &colors[0]);

    // Set up vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 
                         (void*)(vertices.size() * sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);
}
