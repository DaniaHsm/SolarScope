#include "Skybox.hpp"
#include <glm/gtc/type_ptr.hpp>

void Skybox::render(GLuint shader, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) const {
    glDepthFunc(GL_LEQUAL);
    glUseProgram(shader);

    // Remove translation from view matrix for skybox
    glm::mat4 skyboxView = glm::mat4(glm::mat3(viewMatrix));

    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(skyboxView));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
}
