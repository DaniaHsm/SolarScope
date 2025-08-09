#include "include/Camera.hpp"
#include "include/CelestialBody.hpp"
#include "CelestialBody.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

using namespace glm;

void Camera::updateForSelectedPlanet(CelestialBody* selectedBody, float dt) {
    if (!selectedBody) return;

    // Position camera at a good viewing distance from the selected planet
    float viewingDistance = selectedBody->scale.x * 8.0f; // Adjust multiplier as needed
    viewingDistance = std::max(3.0f, viewingDistance);    // Minimum distance

    // Calculate desired camera position (slightly above and behind the planet)
    vec3 targetPosition = selectedBody->position + vec3(0.0f, viewingDistance * 0.3f, viewingDistance);

    // Smooth camera movement (lerp towards target)
    float lerpSpeed = 2.0f * dt; // Adjust speed as needed
    position = mix(position, targetPosition, lerpSpeed);

    // Make camera look at the selected planet
    vec3 directionToPlanet = normalize(selectedBody->position - position);
    lookAt = directionToPlanet;
}

void Camera::updateAngles(float dx, float dy, float dt) {
    const float cameraAngularSpeed = 60.0f;
    horizontalAngle -= dx * cameraAngularSpeed * dt;
    verticalAngle -= dy * cameraAngularSpeed * dt;

    verticalAngle = std::max(-85.0f, std::min(85.0f, verticalAngle));

    float theta = radians(horizontalAngle);
    float phi = radians(verticalAngle);
    lookAt = vec3(cos(phi) * cos(theta), sin(phi), -cos(phi) * sin(theta));
}

void Camera::updatePosition(GLFWwindow* window, float dt) {
    bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
                   glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    float currentCameraSpeed = fastCam ? fastSpeed : speed;
    vec3 cameraSideVector = cross(lookAt, up);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += lookAt * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= lookAt * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= cameraSideVector * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += cameraSideVector * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        position += up * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        position -= up * dt * currentCameraSpeed;
    }
}

glm::mat4 Camera::updateViewMatrix() const
{
    if (firstPerson)
    {
        return glm::lookAt(position, position + lookAt, up);
    }
    else
    {
        // Distance from target in third-person
        float radius = 1.5f;
        glm::vec3 position = this->position - radius * lookAt;
        return glm::lookAt(position, position + lookAt, up);
    }
}
