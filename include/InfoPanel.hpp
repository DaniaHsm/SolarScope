#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include "PlanetInfo.hpp"

class InfoPanel {
public:
    bool visible;
    bool wasIPressed;
    PlanetInfo currentInfo;
    float fadeAlpha;
    GLuint currentTexture;         // Current planet info texture
    std::vector<GLuint> planetTextures; // All planet info textures
    std::vector<std::string> planetNames;    // Planet names for texture mapping

    InfoPanel();
    void loadPlanetTextures();
    void show(const PlanetInfo& info);
    void hide();
    void toggle(const PlanetInfo& info);
    void update(float dt);
    void handleInput(GLFWwindow* window, const PlanetInfo& currentPlanetInfo);
    
    // Renders the semi-transparent background for the info panel
    void renderBackground(GLuint shader, int windowWidth, int windowHeight) const;
    void renderOnScreen(GLuint uiShader, int windowWidth, int windowHeight) const;
};
