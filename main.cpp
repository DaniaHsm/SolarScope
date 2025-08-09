#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLEW_STATIC 1
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

#include "include/models/Mesh.hpp"
#include "include/models/Model.hpp"

#include "include/space_objects/BlackHole.hpp"
#include "include/space_objects/CelestialBody.hpp"
#include "include/space_objects/Comet.hpp"
#include "include/space_objects/PlanetRing.hpp"
#include "include/space_objects/TrailPoint.hpp"

#include "include/utils/GeometryUtils.hpp"
#include "include/utils/ShaderUtils.hpp"
#include "include/utils/SphereUtils.hpp"
#include "include/utils/TextureUtils.hpp"

#include "include/world/Camera.hpp"
#include "include/world/InfoPanel.hpp"
#include "include/world/PlanetInfo.hpp"
#include "include/world/PlanetSelector.hpp"
#include "include/world/ShaderPrograms.hpp"
#include "include/world/Skybox.hpp"
#include "include/world/Window.hpp"

using namespace glm;
using namespace std;

int main(int argc, char *argv[])
{
    // Initialize GLFW and OpenGL
    GLFWwindow *window = Window::initializeGLFW();
    if (!window)
    {
        return -1;
    }

    if (!Window::initializeOpenGL())
    {
        glfwTerminate();
        return -1;
    }

    // Enable OpenGL features
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Setup shaders and camera
    ShaderPrograms shaders = ShaderUtils::setupShaderPrograms();
    Camera camera; // Constructor handles setup

    // Setup projection and view matrices
    mat4 projectionMatrix = glm::perspective(70.0f, 800.0f / 600.0f, 0.01f, 100.0f);

    mat4 viewMatrix = camera.updateViewMatrix();

    GLuint projectionMatrixLocation = glGetUniformLocation(shaders.base, "projectionMatrix");

    GLuint viewMatrixLocation = glGetUniformLocation(shaders.base, "viewMatrix");

    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

    // Create scene objects
    int vao = GeometryUtils::createVertexBufferObject();
    Model duckModel = Model::loadFromFile("models/rubber_duck/scene.gltf");

    // Setup celestial bodies with realistic proportions
    // Using a scale where Earth = 0.3f as base reference

    /// SUN - Center of the system
    CelestialBody sun = CelestialBody::create("textures/planet/sun.jpg",
                                      4.0f,   // Scale
                                      0.0f,   // Orbit radius
                                      0.0f,   // Orbit speed
                                      15.0f);  // Rotation speed
    sun.position = vec3(0.0f, 0.0f, -20.0f);

    // MERCURY - Smallest planet, closest orbit
    CelestialBody mercury = CelestialBody::create("textures/planet/mercury.jpg",
                                                0.11f, // Small size
                                                8.0f,  // Safe distance from sun (was 3.0f)
                                                2.0f,  // Fastest orbital speed
                                                35.0f  // Fast rotation
    );

    // VENUS - Second planet
    CelestialBody venus = CelestialBody::create("textures/planet/venus.jpg",
                                              0.28f, // Venus size
                                              10.0f, // Safe distance from mercury (was 4.5f)
                                              1.6f,  // Orbital speed
                                              -12.0f // Slow retrograde rotation
    );

    // EARTH - Third planet
    CelestialBody earth = CelestialBody::create("textures/planet/earth.jpg",
                                              0.35f, // Earth size
                                              12.0f, // Safe distance from venus (was 6.0f)
                                              1.0f,  // Earth orbital speed reference
                                              20.0f  // Earth rotation speed
    );

    // MOON - Orbits Earth
    CelestialBody moon = CelestialBody::create("textures/planet/moon.jpg",
                                             0.08f, // Small moon size
                                             1.2f,  // Distance from Earth (increased from 1.0f)
                                             4.0f,  // Fast orbit around Earth
                                             5.0f   // Moon rotation
    );

    // MARS - Fourth planet
    CelestialBody mars = CelestialBody::create("textures/planet/mars.jpg",
                                             0.16f, // Mars size
                                             15.0f, // Safe distance from Earth (was 7.5f)
                                             0.8f,  // Slower orbital speed than Earth
                                             18.0f  // Rotation speed
    );

    // JUPITER - Fifth planet, largest
    CelestialBody jupiter = CelestialBody::create("textures/planet/jupiter.jpg",
                                                3.36f, // Large size
                                                20.0f, // Safe distance from Mars (was 10.0f)
                                                0.5f,  // Slower orbital speed
                                                30.0f  // Fast rotation speed
    );

    // SATURN - Sixth planet with rings
    CelestialBody saturn = CelestialBody::create("textures/planet/saturn.jpg",
                                               2.82f, // Large size
                                               36.0f, // Increased distance from Jupiter to accommodate rings
                                               0.35f, // Slow orbital speed
                                               28.0f  // Fast rotation speed
    );

    // URANUS - Seventh planet
    CelestialBody uranus = CelestialBody::create("textures/planet/uranus.jpg",
                                               1.4f,  // Medium size
                                               50.0f, // Increased distance from Saturn to avoid ring collision
                                               0.25f, // Very slow orbital speed
                                               -15.0f // Retrograde rotation
    );

    // NEPTUNE - Outermost planet
    CelestialBody neptune = CelestialBody::create("textures/planet/neptune.jpg",
                                                1.17f, // Medium size
                                                55.0f, // Safe distance from Uranus (was 22.0f)
                                                0.2f,  // Slowest orbital speed
                                                18.0f  // Normal rotation speed
    );

    Comet halleysComet = Comet::create("textures/comet/comet.jpg", vec3(0.0f, 0.0f, -20.0f), 45.0f, 0.85f);

    Comet comet2 = Comet::create("textures/comet/comet.jpg", vec3(0.0f, 0.0f, -20.0f), 25.0f, 0.7f);
    comet2.orbitAngle = 180.0f; // Start on opposite side

    // Create Saturn's rings
    PlanetRing saturnRings = PlanetRing::createSaturnRings();

    // Setup planet selector with detailed information
    vector<CelestialBody *> allBodies =
        {&sun, &mercury, &mars, &venus, &earth, &moon, &neptune, &uranus, &saturn, &jupiter};
    PlanetSelector planetSelector = PlanetSelector::setupWithInfo(allBodies);

    // Add info panel
    InfoPanel infoPanel;
    infoPanel.loadPlanetTextures();


    // Add planet selection mode flag
    bool planetSelectionMode = false;

    // Store reset positions AFTER bodies are created but BEFORE any updates
    BlackHole blackHole = BlackHole::create();

    // Now set up initial orbital positions for normal animation
    mercury.update(sun.position, 0.0f, 0.0f);
    venus.update(sun.position, 45.0f, 0.0f);
    earth.update(sun.position, 90.0f, 0.0f);
    mars.update(sun.position, 135.0f, 0.0f);
    jupiter.update(sun.position, 180.0f, 0.0f);
    saturn.update(sun.position, 225.0f, 0.0f);
    uranus.update(sun.position, 270.0f, 0.0f);
    neptune.update(sun.position, 315.0f, 0.0f);
    moon.update(earth.position, 0.0f, 0.0f);

    // Store these as the RESET positions (what we return to with R key)
    blackHole.resetPositions.clear();
    blackHole.resetScales.clear();
    blackHole.resetPositions.push_back(sun.position);
    blackHole.resetScales.push_back(sun.scale);
    blackHole.resetPositions.push_back(mercury.position);
    blackHole.resetScales.push_back(mercury.scale);
    blackHole.resetPositions.push_back(venus.position);
    blackHole.resetScales.push_back(venus.scale);
    blackHole.resetPositions.push_back(earth.position);
    blackHole.resetScales.push_back(earth.scale);
    blackHole.resetPositions.push_back(moon.position);
    blackHole.resetScales.push_back(moon.scale);
    blackHole.resetPositions.push_back(mars.position);
    blackHole.resetScales.push_back(mars.scale);
    blackHole.resetPositions.push_back(jupiter.position);
    blackHole.resetScales.push_back(jupiter.scale);
    blackHole.resetPositions.push_back(saturn.position);
    blackHole.resetScales.push_back(saturn.scale);
    blackHole.resetPositions.push_back(uranus.position);
    blackHole.resetScales.push_back(uranus.scale);
    blackHole.resetPositions.push_back(neptune.position);
    blackHole.resetScales.push_back(neptune.scale);

    // Setup skybox
    std::vector<std::string> skyboxFaces = {"textures/skybox/1.png",
                                            "textures/skybox/2.png",
                                            "textures/skybox/3.png",
                                            "textures/skybox/4.png",
                                            "textures/skybox/5.png",
                                            "textures/skybox/6.png"};
    Skybox skybox = Skybox::create(skyboxFaces);

    // Initialize animation variables
    float spinningCubeAngle = 0.0f;
    float orbAngle = 0.0f;
    float orbRadius = 2.0f;
    float orbHeight = 1.0f;
    float orbSize = 0.2f;

    // Set up texture uniform for the base shader
    glUseProgram(shaders.base);
    glUniform1i(glGetUniformLocation(shaders.base, "texture1"), 0);

    // Initialize timing and input state
    float lastFrameTime = glfwGetTime();
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
    bool isPaused = false;
    bool wasSpacePressed = false;
    bool comparisonMode = false;
    bool wasCPressed = false;

    // Time control variables
    float timeSpeed = 1.0f; // Normal speed = 1.0, faster = >1.0, slower = <1.0, reverse = negative
    bool wasEqualPressed = false;
    bool wasMinusPressed = false;
    bool wasTPressed = false;

    // Input state tracking for X & R keys
    static bool wasXPressed = false;
    static bool wasRPressed = false;


    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Update timing
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;


        // Handle pause input
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (!wasSpacePressed)
            {
                isPaused = !isPaused;
                wasSpacePressed = true;
            }
        }
        else
        {
            wasSpacePressed = false;
        }

        // Time speed controls
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
        {
            if (!wasEqualPressed)
            {
                timeSpeed *= 1.5f; // Increase speed by 50%
                if (timeSpeed > 10.0f)
                    timeSpeed = 10.0f; // Cap at 10x speed
                std::cout << "Time speed: " << timeSpeed << "x" << std::endl;
                wasEqualPressed = true;
            }
        }
        else
        {
            wasEqualPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
        {
            if (!wasMinusPressed)
            {
                timeSpeed /= 1.5f; // Decrease speed
                if (timeSpeed < -10.0f)
                    timeSpeed = -10.0f; // Cap reverse speed
                std::cout << "Time speed: " << timeSpeed << "x" << std::endl;
                wasMinusPressed = true;
            }
        }
        else
        {
            wasMinusPressed = false;
        }

        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
        {
            if (!wasTPressed)
            {
                timeSpeed *= -1.0f; // Reverse time direction
                std::cout << "Time " << (timeSpeed < 0 ? "reversed" : "forward") << " at " << abs(timeSpeed)
                          << "x speed" << std::endl;
                wasTPressed = true;
            }
        }
        else
        {
            wasTPressed = false;
        }

        // Reset to normal speed
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        {
            timeSpeed = 1.0f;
            std::cout << "Time speed reset to normal" << std::endl;
        }

        // Planet size comparison mode
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            if (!wasCPressed)
            {
                comparisonMode = !comparisonMode;
                if (comparisonMode)
                {
                    std::cout << "Planet size comparison mode ON - Press C again to return to orbit mode" << std::endl;
                }
                else
                {
                    std::cout << "Returned to normal orbit mode" << std::endl;
                }
                wasCPressed = true;
            }
        }
        else
        {
            wasCPressed = false;
        }


        // Handle planet selection with key 3
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            if (!planetSelector.was3Pressed)
            {
                if (!planetSelectionMode)
                {
                    // Enter planet selection mode
                    planetSelectionMode = true;
                    camera.firstPerson = false; // Switch to third person for better planet viewing
                    cout << "Entered planet selection mode. Press 'I' to show planet info. Selected: "
                         << planetSelector.getSelectedName() << endl;
                }
                else
                {
                    // Cycle to next planet
                    planetSelector.nextSelection();
                    // If info panel is currently visible, update it with new planet info
                    if (infoPanel.visible)
                    {
                        infoPanel.show(planetSelector.getSelectedInfo());
                    }
                    cout << "Selected: " << planetSelector.getSelectedName() << " (Press 'I' for info)" << endl;
                }
                planetSelector.was3Pressed = true;
            }
        }
        else
        {
            planetSelector.was3Pressed = false;
        }

        // Exit planet selection mode with key 4
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        {
            if (planetSelectionMode)
            {
                planetSelectionMode = false;
                infoPanel.hide(); // Hide info panel when exiting
                cout << "Exited planet selection mode" << endl;
            }
        }

        // Handle 'I' key for showing/hiding planet info (only in planet selection mode)
        if (planetSelectionMode)
        {
            infoPanel.handleInput(window, planetSelector.getSelectedInfo());
        }

        // Update info panel
        infoPanel.update(dt);

        // X and R key handling for black hole effect
        // X key to activate black hole
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            if (!wasXPressed && !blackHole.active)
            {
                blackHole.active = true;
                blackHole.activationTime = glfwGetTime();
                std::cout << "Black hole activated!" << std::endl;

                // CRITICAL FIX: Capture CURRENT positions when X is pressed, not stored positions
                blackHole.originalPositions.clear();
                blackHole.originalScales.clear();
                blackHole.originalPositions.push_back(sun.position); // Current sun position
                blackHole.originalScales.push_back(sun.scale);
                blackHole.originalPositions.push_back(mercury.position); // Current mercury position
                blackHole.originalScales.push_back(mercury.scale);
                blackHole.originalPositions.push_back(venus.position); // Current venus position
                blackHole.originalScales.push_back(venus.scale);
                blackHole.originalPositions.push_back(earth.position); // Current earth position
                blackHole.originalScales.push_back(earth.scale);
                blackHole.originalPositions.push_back(moon.position); // Current moon position
                blackHole.originalScales.push_back(moon.scale);
                blackHole.originalPositions.push_back(mars.position); // Current mars position
                blackHole.originalScales.push_back(mars.scale);
                blackHole.originalPositions.push_back(jupiter.position); // Current jupiter position
                blackHole.originalScales.push_back(jupiter.scale);
                blackHole.originalPositions.push_back(saturn.position); // Current saturn position
                blackHole.originalScales.push_back(saturn.scale);
                blackHole.originalPositions.push_back(uranus.position); // Current uranus position
                blackHole.originalScales.push_back(uranus.scale);
                blackHole.originalPositions.push_back(neptune.position); // Current neptune position
                blackHole.originalScales.push_back(neptune.scale);

                wasXPressed = true;
            }
        }
        else
        {
            wasXPressed = false;
        }

        // R key to reset
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            if (!wasRPressed)
            {
                blackHole.active = false;
                blackHole.strength = 0.0f;
                std::cout << "Black hole reset!" << std::endl;

                // Reset all bodies to normal orbital positions (not the X-pressed positions)
                sun.position = blackHole.resetPositions[0];
                sun.scale = blackHole.resetScales[0];
                mercury.position = blackHole.resetPositions[1];
                mercury.scale = blackHole.resetScales[1];
                venus.position = blackHole.resetPositions[2];
                venus.scale = blackHole.resetScales[2];
                earth.position = blackHole.resetPositions[3];
                earth.scale = blackHole.resetScales[3];
                moon.position = blackHole.resetPositions[4];
                moon.scale = blackHole.resetScales[4];
                mars.position = blackHole.resetPositions[5];
                mars.scale = blackHole.resetScales[5];
                jupiter.position = blackHole.resetPositions[6];
                jupiter.scale = blackHole.resetScales[6];
                saturn.position = blackHole.resetPositions[7];
                saturn.scale = blackHole.resetScales[7];
                uranus.position = blackHole.resetPositions[8];
                uranus.scale = blackHole.resetScales[8];
                neptune.position = blackHole.resetPositions[9];
                neptune.scale = blackHole.resetScales[9];

                wasRPressed = true;
            }
        }
        else
        {
            wasRPressed = false;
        }

        float animationDt = isPaused ? 0.0f : dt * timeSpeed;

        // Update camera for planet selection mode
        if (planetSelectionMode)
        {
            CelestialBody *selectedBody = planetSelector.getSelectedBody();
            camera.updateForSelectedPlanet(selectedBody, dt);
        }

        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update view matrix
        mat4 viewMatrix = camera.updateViewMatrix();
        GLuint viewMatrixLocation = glGetUniformLocation(shaders.base, "viewMatrix");
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

        // Render skybox
        skybox.render(shaders.skybox, viewMatrix, projectionMatrix);

        // Setup base shader for scene rendering
        glUseProgram(shaders.base);

        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

        glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        glBindVertexArray(vao);

        // Update and render spinning duck (third-person view only)
        spinningCubeAngle += 180.0f * dt;
        if (!camera.firstPerson && !planetSelectionMode)
        {
            GLuint worldMatrixLocation = glGetUniformLocation(shaders.base, "worldMatrix");

            mat4 spinningCubeWorldMatrix = translate(mat4(1.0f), camera.position + vec3(0.0f, -0.2f, 0.0f)) *
                                           rotate(mat4(1.0f), radians(spinningCubeAngle), vec3(0.0f, 1.0f, 0.0f)) *
                                           rotate(mat4(1.0f), radians(1.0f), vec3(0.0f, 0.0f, 1.0f)) *
                                           scale(mat4(1.0f), vec3(0.0006f, 0.0006f, 0.0006f));

            glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &spinningCubeWorldMatrix[0][0]);

            if (!duckModel.meshes.empty())
            {
                glDisable(GL_CULL_FACE);
                duckModel.Draw(shaders.base);
                glEnable(GL_CULL_FACE);
            }
        }

        // Update celestial body positions and handle black hole effect
        orbAngle += 20.0f * animationDt;
        vec3 sunPosition = vec3(0.0f, 0.0f, -20.0f);

        // FIXED: Proper black hole effect implementation
        if (blackHole.active)
        {
            float elapsed = glfwGetTime() - blackHole.activationTime;
            float effectDuration = 6.0f; // 6 second effect for better visibility
            blackHole.strength = std::min(1.0f, elapsed / effectDuration);

            // Apply black hole effect to all bodies - shrink to COMPLETELY INVISIBLE
            float shrinkFactor =
                std::max(0.0f, 1.0f - blackHole.strength); // Goes from 1.0 to 0.0 (completely invisible)

            // Linear interpolation function
            auto lerp = [](const vec3 &a, const vec3 &b, float t) -> vec3 { return a + t * (b - a); };

            // Apply effect to ALL bodies including the sun - they all go to the black hole center
            sun.position = lerp(blackHole.originalPositions[0], blackHole.position, blackHole.strength);
            sun.scale = blackHole.originalScales[0] * shrinkFactor;

            mercury.position = lerp(blackHole.originalPositions[1], blackHole.position, blackHole.strength);
            mercury.scale = blackHole.originalScales[1] * shrinkFactor;

            venus.position = lerp(blackHole.originalPositions[2], blackHole.position, blackHole.strength);
            venus.scale = blackHole.originalScales[2] * shrinkFactor;

            earth.position = lerp(blackHole.originalPositions[3], blackHole.position, blackHole.strength);
            earth.scale = blackHole.originalScales[3] * shrinkFactor;

            moon.position = lerp(blackHole.originalPositions[4], blackHole.position, blackHole.strength);
            moon.scale = blackHole.originalScales[4] * shrinkFactor;

            mars.position = lerp(blackHole.originalPositions[5], blackHole.position, blackHole.strength);
            mars.scale = blackHole.originalScales[5] * shrinkFactor;

            jupiter.position = lerp(blackHole.originalPositions[6], blackHole.position, blackHole.strength);
            jupiter.scale = blackHole.originalScales[6] * shrinkFactor;

            saturn.position = lerp(blackHole.originalPositions[7], blackHole.position, blackHole.strength);
            saturn.scale = blackHole.originalScales[7] * shrinkFactor;

            uranus.position = lerp(blackHole.originalPositions[8], blackHole.position, blackHole.strength);
            uranus.scale = blackHole.originalScales[8] * shrinkFactor;

            neptune.position = lerp(blackHole.originalPositions[9], blackHole.position, blackHole.strength);
            neptune.scale = blackHole.originalScales[9] * shrinkFactor;

            // Don't do normal orbital updates during black hole effect - COMPLETELY override positions
        }
        else
        {
            // Check if we're in comparison mode
            if (comparisonMode)
            {
                // Position planets in a straight line by size: smallest to largest
                // Position sun slightly to the side so it doesn't block planets
                sun.position = vec3(-15.0f, 0.0f, -20.0f);

                // Line up planets by size from smallest to largest, moving away from sun
                // Position them so sun light can reach all of them (no shadows)

                float baseSpacing = 8.0f;    // Base distance between each planet
                float saturnExtra = 4.0f;    // Extra spacing around Saturn for rings
                float startDistance = 5.0f;   // Distance from sun position to first planet
                
                // Calculate positions with extra space around Saturn
                mercury.position = vec3(startDistance, 0, -20);               // 0.11f - Smallest
                mars.position = vec3(startDistance + baseSpacing * 1, 0, -20);    // 0.16f - Mars is smaller than Earth!
                venus.position = vec3(startDistance + baseSpacing * 2, 0, -20);   // 0.28f
                earth.position = vec3(startDistance + baseSpacing * 3, 0, -20);   // 0.3f
                moon.position = vec3(startDistance + baseSpacing * 3.3f, 2, -20); // Moon near Earth
                neptune.position = vec3(startDistance + baseSpacing * 4, 0, -20); // 1.17f - Neptune is smaller than Uranus!
                uranus.position = vec3(startDistance + baseSpacing * 5, 0, -20);  // 1.2f
                saturn.position = vec3(startDistance + baseSpacing * 6 + saturnExtra, 0, -20);  // 2.82f - Extra space for rings
                jupiter.position = vec3(startDistance + baseSpacing * 7 + saturnExtra * 2.5, 0, -20); // 3.36f - Largest

                // Still allow rotation in comparison mode
                sun.rotationAngle += sun.rotationSpeed * animationDt;
                mercury.rotationAngle += mercury.rotationSpeed * animationDt;
                venus.rotationAngle += venus.rotationSpeed * animationDt;
                earth.rotationAngle += earth.rotationSpeed * animationDt;
                mars.rotationAngle += mars.rotationSpeed * animationDt;
                jupiter.rotationAngle += jupiter.rotationSpeed * animationDt;
                saturn.rotationAngle += saturn.rotationSpeed * animationDt;
                uranus.rotationAngle += uranus.rotationSpeed * animationDt;
                neptune.rotationAngle += neptune.rotationSpeed * animationDt;
                moon.rotationAngle += moon.rotationSpeed * animationDt;
            }
            else
            {
                // Normal celestial body updates only when black hole is not active
                sun.update(sun.position, orbAngle, animationDt);
                mercury.update(sun.position, orbAngle, animationDt);
                venus.update(sun.position, orbAngle, animationDt);
                earth.update(sun.position, orbAngle, animationDt);
                mars.update(sun.position, orbAngle, animationDt);
                jupiter.update(sun.position, orbAngle, animationDt);
                saturn.update(sun.position, orbAngle, animationDt);
                uranus.update(sun.position, orbAngle, animationDt);
                neptune.update(sun.position, orbAngle, animationDt);
                moon.update(earth.position, orbAngle, animationDt);
            }
        }
        // Update comets
		halleysComet.update(animationDt, sun.position);
        comet2.update(animationDt, sun.position);

        // Collect all planet positions and radii for shadow calculations
        vector<vec3> planetPositions;
        vector<float> planetRadii;
        
        if (!comparisonMode) {
            planetPositions = {
                mercury.position,
                venus.position,
                earth.position,
                mars.position,
                jupiter.position,
                saturn.position,
                uranus.position,
                neptune.position,
                moon.position
            };
            planetRadii = {
                mercury.scale.x,
                venus.scale.x,
                earth.scale.x,
                mars.scale.x,
                jupiter.scale.x,
                saturn.scale.x,
                uranus.scale.x,
                neptune.scale.x,
                moon.scale.x
            };
        }

        // Render all celestial bodies in order from sun outward - BUT ONLY IF VISIBLE
        // Check if each body is large enough to be visible (scale > 0.01f means visible)

        if (sun.scale.x > 0.01f)
        {
            sun.render(shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, true);
        }
        if (mercury.scale.x > 0.01f)
        {
            mercury.render(shaders.orb,
                         viewMatrix,
                         projectionMatrix,
                         sun.position,
                         camera.position,
                         false,
                         planetPositions,
                         planetRadii);
        }
        if (venus.scale.x > 0.01f)
        {
            venus.render(shaders.orb,
                        viewMatrix,
                        projectionMatrix,
                        sun.position,
                        camera.position,
                        false,
                        planetPositions,
                        planetRadii);
        }
        if (earth.scale.x > 0.01f)
        {
            earth.render(shaders.orb,
                        viewMatrix,
                        projectionMatrix,
                        sun.position,
                        camera.position,
                        false,
                        planetPositions,
                        planetRadii);
        }
        if (mars.scale.x > 0.01f)
        {
            mars.render(shaders.orb,
                       viewMatrix,
                       projectionMatrix,
                       sun.position,
                       camera.position,
                       false,
                       planetPositions,
                       planetRadii);
        }
        if (jupiter.scale.x > 0.01f)
        {
            jupiter.render(shaders.orb,
                         viewMatrix,
                         projectionMatrix,
                         sun.position,
                         camera.position,
                         false,
                         planetPositions,
                         planetRadii);
        }
        if (saturn.scale.x > 0.01f)
        {
            saturn.render(shaders.orb,
                         viewMatrix,
                         projectionMatrix,
                         sun.position,
                         camera.position,
                         false,
                         planetPositions,
                         planetRadii);
        }
        // Render Saturn's rings immediately after Saturn only if Saturn is visible
        if (saturn.scale.x > 0.01f) {
            saturnRings.render(saturn, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position);
        }
        if (uranus.scale.x > 0.01f)
        {
            uranus.render(shaders.orb,
                         viewMatrix,
                         projectionMatrix,
                         sun.position,
                         camera.position,
                         false,
                         planetPositions,
                         planetRadii);
        }
        if (neptune.scale.x > 0.01f)
        {
            neptune.render(shaders.orb,
                          viewMatrix,
                          projectionMatrix,
                          sun.position,
                          camera.position,
                          false,
                          planetPositions,
                          planetRadii);
        }
        if (moon.scale.x > 0.01f)
        {
            moon.render(shaders.orb,
                       viewMatrix,
                       projectionMatrix,
                       sun.position,
                       camera.position,
                       false,
                       planetPositions,
                       planetRadii);
        }

        // Render comet trails first (so they appear behind comet heads)
        halleysComet.renderTrail(shaders.base, viewMatrix, projectionMatrix);
        
        comet2.renderTrail(shaders.base, viewMatrix, projectionMatrix);        // Render comet heads
        halleysComet.body.render(shaders.orb,
                                  viewMatrix,
                                  projectionMatrix,
                                  sun.position,
                                  camera.position,
                                  false,
                                  planetPositions,
                                  planetRadii);
        comet2.body.render(shaders.orb,
                          viewMatrix,
                          projectionMatrix,
                          sun.position,
                          camera.position,
                          false,
                          planetPositions,
                          planetRadii);

        // Render selection indicator if in planet selection mode
        if (planetSelectionMode)
        {
            CelestialBody *selectedBody = planetSelector.getSelectedBody();
            planetSelector.renderSelectionIndicator(shaders.selection, viewMatrix, projectionMatrix);
        }

        // Render info panel if visible
        if (planetSelectionMode && infoPanel.visible)
        {
            infoPanel.renderOnScreen(shaders.ui, 800, 600);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Handle keyboard input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        // Only allow manual camera mode switching when not in planet selection mode
        if (!planetSelectionMode)
        {
            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            {
                camera.firstPerson = true;
            }
            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            {
                camera.firstPerson = false;
            }
        }

        // Update camera from mouse input (only when not in planet selection mode)
        if (!planetSelectionMode)
        {
            double mousePosX, mousePosY;
            glfwGetCursorPos(window, &mousePosX, &mousePosY);
            double dx = mousePosX - lastMousePosX;
            double dy = mousePosY - lastMousePosY;
            lastMousePosX = mousePosX;
            lastMousePosY = mousePosY;

            camera.updateAngles(static_cast<float>(dx), static_cast<float>(dy), dt);
            camera.updatePosition(window, dt);

            // Handle arrow key camera control
            const float arrowLookSpeed = 60.0f;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            {
                camera.horizontalAngle += arrowLookSpeed * dt;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            {
                camera.horizontalAngle -= arrowLookSpeed * dt;
            }
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            {
                camera.verticalAngle += arrowLookSpeed * dt;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            {
                camera.verticalAngle -= arrowLookSpeed * dt;
            }

            // Update camera look direction based on angles
            float theta = radians(camera.horizontalAngle);
            float phi = radians(camera.verticalAngle);
            camera.lookAt = vec3(cos(phi) * cos(theta), sin(phi), -cos(phi) * sin(theta));
        }
    }

    // Cleanup
    glfwTerminate();
    return 0;
}
