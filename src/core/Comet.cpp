#include "Comet.hpp"
#include "CelestialBodyFactory.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>

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
