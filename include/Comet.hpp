#pragma once
#include <vector>
#include <GL/glew.h>
#include "CelestialBody.hpp"
#include "TrailPoint.hpp"

class Comet {
public:
    CelestialBody body;            // Reuse existing celestial body for the head
    std::vector<TrailPoint> trail; // Trail points
    float orbitAngle;              // Current angle in elliptical orbit
    float eccentricity;            // How elliptical the orbit is (0 = circle, 0.9 = very elliptical)
    float semiMajorAxis;           // Size of the orbit
    glm::vec3 orbitCenter;         // Center point of orbit
    GLuint trailVAO;               // VAO for trail rendering
    GLuint trailVBO;               // VBO for trail vertices
    int maxTrailPoints;            // Maximum trail length
    float lastTrailUpdate;         // Time tracking for trail updates

    // Factory method to create a comet
    static Comet create(const char* texturePath,
                       const glm::vec3& orbitCenter,
                       float semiMajorAxis,
                       float eccentricity);

    // Update comet's position and trail
    void update(float dt, const glm::vec3& sunPosition);

    void updateTrail(float currentTime, const glm::vec3& sunPosition);
    void updateTrailVBO();

    // Render the comet's trail
    void renderTrail(GLuint shader, 
                    const glm::mat4& viewMatrix, 
                    const glm::mat4& projectionMatrix) const;
};
