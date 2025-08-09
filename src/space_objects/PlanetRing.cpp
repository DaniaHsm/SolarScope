#include "include/space_objects/PlanetRing.hpp"
#include "include/utils/SphereUtils.hpp"
#include "include/utils/TextureUtils.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

using namespace glm;

PlanetRing PlanetRing::createSaturnRings() {
    PlanetRing ring;

    // Create ring geometry (simplified as a flat disk with hole)
    std::vector<vec3> vertices;
    std::vector<vec2> uvs;
    std::vector<unsigned int> indices;

    float innerRadius = 1.2f; // Inner edge of rings
    float outerRadius = 2.0f; // Outer edge of rings
    int segments = 64;        // Number of segments around the ring

    // Generate ring vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * pi<float>();
        float cosA = cos(angle);
        float sinA = sin(angle);

        // Inner vertex
        vertices.push_back(vec3(innerRadius * cosA, 0.0f, innerRadius * sinA));
        uvs.push_back(vec2(0.0f, (float)i / segments));

        // Outer vertex
        vertices.push_back(vec3(outerRadius * cosA, 0.0f, outerRadius * sinA));
        uvs.push_back(vec2(1.0f, (float)i / segments));
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

    ring.vao = SphereUtils::setupSphereBuffers(vertices, uvs, indices);
    ring.indexCount = indices.size();
    ring.texture = TextureUtils::loadTexture("textures/planet/saturn_rings.png");
    ring.innerRadius = innerRadius;
    ring.outerRadius = outerRadius;

    return ring;
}

void PlanetRing::render(const CelestialBody& planet,
                         GLuint shader,
                         const mat4& viewMatrix,
                         const mat4& projectionMatrix,
                         const vec3& lightPos,
                         const vec3& viewPos) const {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE); // Rings should be visible from both sides

    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader, "isSun"), 0); // Rings are not the sun

    // Position rings at planet location and scale them with the planet while maintaining ring proportions
    mat4 worldMatrix = translate(mat4(1.0f), planet.position) *
                       rotate(mat4(1.0f), radians(-10.0f), vec3(1.0f, 0.0f, 0.0f)) *
                       scale(mat4(1.0f), vec3(planet.scale.x * 1.5f, planet.scale.y, planet.scale.z * 1.5f));

    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);

    // Set lighting uniforms
    glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, &viewPos[0]);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}
