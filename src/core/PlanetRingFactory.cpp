#include "PlanetRingFactory.hpp"
#include "TextureLoader.hpp"
#include "SphereGeometry.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

PlanetRing PlanetRingFactory::createSaturnRings() {
    PlanetRing ring;

    // Create ring geometry (simplified as a flat disk with hole)
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;

    float innerRadius = 1.2f; // Inner edge of rings
    float outerRadius = 2.0f; // Outer edge of rings
    int segments = 64;        // Number of segments around the ring

    // Generate ring vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * glm::pi<float>();
        float cosA = cos(angle);
        float sinA = sin(angle);

        // Inner vertex
        vertices.push_back(glm::vec3(innerRadius * cosA, 0.0f, innerRadius * sinA));
        uvs.push_back(glm::vec2(0.0f, (float)i / segments));

        // Outer vertex
        vertices.push_back(glm::vec3(outerRadius * cosA, 0.0f, outerRadius * sinA));
        uvs.push_back(glm::vec2(1.0f, (float)i / segments));
    }

    // Generate indices for ring triangles
    for (int i = 0; i < segments * 2; i += 2) {
        // First triangle
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);

        // Second triangle
        indices.push_back(i + 1);
        indices.push_back(i + 3);
        indices.push_back(i + 2);
    }

    ring.vao = SphereGeometry::setupSphereBuffers(vertices, uvs, indices);
    ring.indexCount = indices.size();
    ring.texture = TextureLoader::loadTexture("textures/saturn_rings.png");
    ring.innerRadius = innerRadius;
    ring.outerRadius = outerRadius;

    return ring;
}
