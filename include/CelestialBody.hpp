#pragma once
#define GLEW_STATIC 1
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

struct CelestialBody {
    GLuint vao;              // Vertex Array Object for the sphere
    GLuint texture;          // Body's surface texture
    unsigned int indexCount; // Number of indices for rendering
    glm::vec3 position;      // Current position in space
    glm::vec3 scale;         // Size of the celestial body
    float rotationAngle;     // Current rotation around its axis
    float rotationSpeed;     // Speed of rotation around its axis
    float orbitRadius;       // Distance from the center of orbit
    float orbitSpeed;        // Speed of orbital movement

    // Update the celestial body's position and rotation
    void update(const glm::vec3& centerPosition, float baseAngle, float dt);
    
    // Get the world transformation matrix for rendering
    glm::mat4 getWorldMatrix() const;

    // Render the celestial body
    void render(GLuint shader,
		const glm::mat4 &viewMatrix,
		const glm::mat4 &projectionMatrix,
		const glm::vec3 &lightPos,
		const glm::vec3 &viewPos,
		bool isSun = false,
const std::vector<glm::vec3> &allPlanetPositions = std::vector<glm::vec3>(),
    	const std::vector<float> &allPlanetRadii = std::vector<float>()
	) const;
};
