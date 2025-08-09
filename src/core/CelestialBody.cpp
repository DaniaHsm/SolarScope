#include "CelestialBody.hpp"
#include "SphereGeometry.hpp"
#include "TextureLoader.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

CelestialBody CelestialBody::create(const char* texturePath,
                                   float scale,
                                   float orbitRadius,
                                   float orbitSpeed,
                                   float rotationSpeed) {
    CelestialBody body;
    body.scale = glm::vec3(scale);
    body.position = glm::vec3(0.0f);
    body.orbitRadius = orbitRadius;
    body.orbitSpeed = orbitSpeed;
    body.rotationSpeed = rotationSpeed;
    body.rotationAngle = 0.0f;

    body.vao = SphereGeometry::createTexturedSphereVAO(40, 40, body.indexCount);
    body.texture = TextureLoader::loadTexture(texturePath);

    return body;
}

void CelestialBody::render(GLuint shader,
                           const glm::mat4& viewMatrix,
                           const glm::mat4& projectionMatrix,
                           const glm::vec3& lightPos,
                           const glm::vec3& viewPos,
                           bool isSun,
                           const std::vector<glm::vec3>& allPlanetPositions,
                           const std::vector<float>& allPlanetRadii) const {
    // Disable culling for celestial bodies to ensure correct appearance
    glDisable(GL_CULL_FACE);
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader, "isSun"), isSun ? 1 : 0);

    // Pass planet data for shadow calculations
    if (!isSun && !allPlanetPositions.empty()) {
        glUniform3fv(glGetUniformLocation(shader, "planetPositions"),
                     allPlanetPositions.size(),
                     &allPlanetPositions[0][0]);
        glUniform1fv(glGetUniformLocation(shader, "planetRadii"), allPlanetRadii.size(), &allPlanetRadii[0]);
        glUniform1i(glGetUniformLocation(shader, "numPlanets"), allPlanetPositions.size());
    }

    glm::mat4 worldMatrix = getWorldMatrix();

    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);

    glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, &viewPos[0]);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

    // Re-enable culling after rendering celestial bodies
    glEnable(GL_CULL_FACE);
}

void CelestialBody::update(const glm::vec3& centerPosition, float baseAngle, float dt) {
    rotationAngle += rotationSpeed * dt;

    float orbitAngle = baseAngle * orbitSpeed;
    position = centerPosition + 
               glm::vec3(orbitRadius * cos(glm::radians(orbitAngle)), 
                        0.0f, 
                        orbitRadius * sin(glm::radians(orbitAngle)));
}

glm::mat4 CelestialBody::getWorldMatrix() const {
    return glm::translate(glm::mat4(1.0f), position) *
           glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)) *
           glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * 
           glm::scale(glm::mat4(1.0f), scale);
}
