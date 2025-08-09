#pragma once
#define GLEW_STATIC 1
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class CelestialBody;  // Forward declaration

class Camera {
public:
    glm::vec3 position;         // Camera position in 3D space
    glm::vec3 lookAt;           // Direction the camera is facing
    glm::vec3 up;              // Up vector for camera orientation
    float speed;               // Normal movement speed
    float fastSpeed;           // Sprint movement speed
    float horizontalAngle;     // Left/right rotation
    float verticalAngle;       // Up/down rotation
    bool firstPerson;          // Toggle between first/third person

    // Constructor with default values can be added here
    Camera() : 
        position(glm::vec3(0.0f, 2.0f, 5.0f)),
        lookAt(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        speed(6.0f),
        fastSpeed(10.0f),
        horizontalAngle(90.0f),
        verticalAngle(0.0f),
        firstPerson(true)
    {}

    // Update camera position for selected planet view
    void updateForSelectedPlanet(class CelestialBody* selectedBody, float dt);
    
    // Update camera angles based on mouse movement
    void updateAngles(float dx, float dy, float dt);
    
    // Update camera position based on keyboard input
    void updatePosition(class GLFWwindow* window, float dt);
    
    // Get view matrix based on camera state
    glm::mat4 updateViewMatrix() const;
};
