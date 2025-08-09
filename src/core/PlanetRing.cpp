#include "PlanetRing.hpp"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

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

    // Position rings at planet location
    mat4 worldMatrix = translate(mat4(1.0f), planet.position) *
                       rotate(mat4(1.0f), radians(-10.0f), vec3(1.0f, 0.0f, 0.0f)) *
                       scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));

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
