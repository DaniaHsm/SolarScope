#include "include/world/InfoPanel.hpp"
#include "include/utils/TextureUtils.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

InfoPanel::InfoPanel() : visible(false), wasIPressed(false), fadeAlpha(0.0f), currentTexture(0) {}

void InfoPanel::loadPlanetTextures() {
    planetNames = {
        "sun",
        "mercury",
        "mars",
        "venus",
        "earth",
        "moon",
        "neptune",
        "uranus",
        "saturn",
        "jupiter",
    };

    for (const std::string& planetName : planetNames) {
        std::string texturePath = "textures/planet_info/" + planetName + "_info.png";
        std::cout << "Attempting to load: " << texturePath << std::endl;

        GLuint texture = TextureUtils::loadTexture(texturePath.c_str());
        if (texture == 0) {
            std::cout << "Warning: Failed to load planet info texture for " << planetName << std::endl;
            if (texture == 0) {
                texture = TextureUtils::loadTexture("textures/planet/sun.jpg");
            }
        } else {
            std::cout << "Successfully loaded texture for " << planetName << " with ID: " << texture << std::endl;
        }
        planetTextures.push_back(texture);
    }
}

void InfoPanel::show(const PlanetInfo& info) {
    currentInfo = info;
    visible = true;
    fadeAlpha = 1.0f;

    for (size_t i = 0; i < planetNames.size(); i++) {
        std::string infoNameLower = info.name;
        std::transform(infoNameLower.begin(), infoNameLower.end(), infoNameLower.begin(), ::tolower);

        if (infoNameLower == planetNames[i]) {
            currentTexture = planetTextures[i];
            std::cout << "Displaying " << info.name << " information on screen" << std::endl;
            return;
        }
    }

    if (!planetTextures.empty()) {
        currentTexture = planetTextures[0];
    }
}

void InfoPanel::hide() {
    visible = false;
}

void InfoPanel::renderBackground(GLuint shader, int windowWidth, int windowHeight) const {
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST); // Render on top of everything

    glUseProgram(shader);

    // Set up orthographic projection for 2D overlay
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f); // Identity

    // Panel dimensions and position (top-right corner)
    float panelWidth = 300.0f;
    float panelHeight = 180.0f;
    float margin = 20.0f;
    float panelX = windowWidth - panelWidth - margin;
    float panelY = windowHeight - panelHeight - margin;

    // Create a simple quad for the background
    float vertices[] = {
        // Positions        // Colors (dark blue with transparency)
        panelX, panelY, 0.1f, 0.1f, 0.3f,             // Bottom-left
        panelX + panelWidth, panelY, 0.1f, 0.1f, 0.3f,         // Bottom-right
        panelX + panelWidth, panelY + panelHeight, 0.1f, 0.1f, 0.3f,  // Top-right
        panelX, panelY + panelHeight, 0.1f, 0.1f, 0.3f         // Top-left
    };

    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };

    static GLuint VAO = 0, VBO = 0, EBO = 0;

    // Create buffers if not already created
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set matrices
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &orthoProjection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);

    // Set alpha for fading
    glUniform1f(glGetUniformLocation(shader, "alpha"), fadeAlpha * 0.8f); // Semi-transparent

    // Draw the quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void InfoPanel::renderOnScreen(GLuint uiShader, int windowWidth, int windowHeight) const {
    if (fadeAlpha <= 0.0f)
        return;

    // Show default message if no planet is selected or no texture
    if (currentTexture == 0) {
        std::cout << "Info Panel: No planet selected. Press '3' to enter planet selection mode." << std::endl;
        return;
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(uiShader);

    // Set up orthographic projection for 2D overlay
    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);

    // Panel dimensions and position (top-right corner)
    float panelWidth = 400.0f;
    float panelHeight = 250.0f;
    float margin = 20.0f;
    float panelX = windowWidth - panelWidth - margin;
    float panelY = windowHeight - panelHeight - margin;

    // Create textured quad vertices
    float vertices[] = {
        // Positions        // Texture Coords
        panelX, panelY, 0.0f, 1.0f,                        // Bottom-left
        panelX + panelWidth, panelY, 1.0f, 1.0f,          // Bottom-right
        panelX + panelWidth, panelY + panelHeight, 1.0f, 0.0f,  // Top-right
        panelX, panelY + panelHeight, 0.0f, 0.0f          // Top-left
    };

    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };

    static GLuint VAO = 0, VBO = 0, EBO = 0;

    // Create buffers if not already created
    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Bind the planet info texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTexture);
    glUniform1i(glGetUniformLocation(uiShader, "ourTexture"), 0);

    // Set projection matrix and alpha
    glUniformMatrix4fv(glGetUniformLocation(uiShader, "projection"), 1, GL_FALSE, &orthoProjection[0][0]);
    glUniform1f(glGetUniformLocation(uiShader, "alpha"), fadeAlpha);

    // Draw the textured quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void InfoPanel::toggle(const PlanetInfo& info) {
    if (visible) {
        hide();
    } else {
        show(info);
    }
}

void InfoPanel::update(float dt) {
    if (visible && fadeAlpha < 1.0f) {
        fadeAlpha = std::min(1.0f, fadeAlpha + dt * 3.0f);
    } else if (!visible && fadeAlpha > 0.0f) {
        fadeAlpha = std::max(0.0f, fadeAlpha - dt * 3.0f);
    }
}

void InfoPanel::handleInput(GLFWwindow* window, const PlanetInfo& currentPlanetInfo) {
    bool iPressed = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;

    if (iPressed && !wasIPressed) {
        toggle(currentPlanetInfo);
        std::cout << (visible ? "Planet info shown on screen" : "Planet info hidden") << std::endl;
    }

    wasIPressed = iPressed;
}
