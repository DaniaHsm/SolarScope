#include "include/world/PlanetSelector.hpp"
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

PlanetSelector::PlanetSelector() : selectedIndex(0), was3Pressed(false) {}

PlanetSelector PlanetSelector::setupWithInfo(std::vector<CelestialBody*>& allBodies) {
    PlanetSelector planetSelector;

    // Sun
    planetSelector.addCelestialBody(allBodies[0],
                                    "Sun",
                                    PlanetInfo("Sun",
                                               "Our stellar powerhouse",
                                               "Temperature: 5,778K surface, 15M K core",
                                               "Mass: 99.86% of entire solar system",
                                               "Powers all life through fusion"));

    // Mercury
    planetSelector.addCelestialBody(allBodies[1],
                                    "Mercury",
                                    PlanetInfo("Mercury",
                                               "Smallest and fastest planet",
                                               "Orbital period: 88 Earth days",
                                               "Temp: 427°C day, -173°C night",
                                               "No atmosphere or moons"));

    // Venus
    planetSelector.addCelestialBody(allBodies[2],
                                    "Venus",
                                    PlanetInfo("Venus",
                                               "Hottest planet with toxic air",
                                               "Surface temp: 462°C (hotter than Mercury)",
                                               "Atmosphere: 96% CO2, crushing pressure",
                                               "Rotates backward (retrograde)"));

    // Earth
    planetSelector.addCelestialBody(allBodies[3],
                                    "Earth",
                                    PlanetInfo("Earth",
                                               "Our beautiful blue marble",
                                               "71% surface covered by water",
                                               "Perfect distance for liquid water",
                                               "Protected by magnetic field"));

    // Moon
    planetSelector.addCelestialBody(allBodies[4],
                                    "Moon",
                                    PlanetInfo("Moon",
                                               "Earth's loyal companion",
                                               "Always shows same face to Earth",
                                               "Created Earth's 24-hour day cycle",
                                               "Made from rock blasted from Earth"));

    // Mars
    planetSelector.addCelestialBody(allBodies[5],
                                    "Mars",
                                    PlanetInfo("Mars",
                                               "The Red Planet, our next home",
                                               "Olympus Mons volcano: 21km high",
                                               "Has polar ice caps and seasons",
                                               "Day: 24h 37min (like Earth)"));

    // Jupiter
    planetSelector.addCelestialBody(allBodies[6],
                                    "Jupiter",
                                    PlanetInfo("Jupiter",
                                               "Giant protector with Great Red Spot",
                                               "Mass: 2.5x all other planets combined",
                                               "Great Red Spot: storm larger than Earth",
                                               "Has 95 moons including 4 major ones"));

    // Saturn
    planetSelector.addCelestialBody(allBodies[7],
                                    "Saturn",
                                    PlanetInfo("Saturn",
                                               "Ringed beauty, less dense than water",
                                               "Density: 0.69 g/cm³ (would float!)",
                                               "Rings made of ice and rock particles",
                                               "Moon Titan has thick atmosphere"));

    // Uranus
    planetSelector.addCelestialBody(allBodies[8],
                                    "Uranus",
                                    PlanetInfo("Uranus",
                                               "Tilted ice giant on its side",
                                               "Rotates on side (98° axial tilt)",
                                               "Made of water, methane & ammonia ice",
                                               "Has faint rings found in 1977"));

    // Neptune
    planetSelector.addCelestialBody(allBodies[9],
                                    "Neptune",
                                    PlanetInfo("Neptune",
                                               "Windiest planet with supersonic storms",
                                               "Wind speeds: up to 2,100 km/h",
                                               "Takes 165 Earth years to orbit Sun",
                                               "Blue color from methane gas"));

    return planetSelector;
}

void PlanetSelector::renderSelectionIndicator(GLuint shader,
                                            const mat4& viewMatrix,
                                            const mat4& projectionMatrix) const {
    CelestialBody* selectedBody = celestialBodies[selectedIndex];
    if (!selectedBody) return;

    glUseProgram(shader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
    glLineWidth(3.0f);                         // Thick lines

    // Create a slightly larger sphere around the selected planet
    float indicatorScale = selectedBody->scale.x * 1.5f;
    mat4 worldMatrix = translate(mat4(1.0f), selectedBody->position) * scale(mat4(1.0f), vec3(indicatorScale));

    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);

    // Set white color for the selection indicator
    vec3 selectionColor = vec3(1.0f, 1.0f, 1.0f); // White
    glUniform3fv(glGetUniformLocation(shader, "selectionColor"), 1, &selectionColor[0]);

    // Render the wireframe sphere
    glBindVertexArray(selectedBody->vao);
    glDrawElements(GL_TRIANGLES, selectedBody->indexCount, GL_UNSIGNED_INT, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Back to solid mode
    glLineWidth(1.0f);                         // Reset line width
}

void PlanetSelector::addCelestialBody(CelestialBody* body, const std::string& name, const PlanetInfo& info) {
    celestialBodies.push_back(body);
    celestialNames.push_back(name);
    planetInfos.push_back(info);
}

void PlanetSelector::nextSelection() {
    selectedIndex = (selectedIndex + 1) % celestialBodies.size();
    std::cout << "Selected: " << celestialNames[selectedIndex] << std::endl;
}

CelestialBody* PlanetSelector::getSelectedBody() {
    if (selectedIndex < celestialBodies.size()) {
        return celestialBodies[selectedIndex];
    }
    return nullptr;
}

std::string PlanetSelector::getSelectedName() {
    if (selectedIndex < celestialNames.size()) {
        return celestialNames[selectedIndex];
    }
    return "Unknown";
}

PlanetInfo PlanetSelector::getSelectedInfo() {
    if (selectedIndex < planetInfos.size()) {
        return planetInfos[selectedIndex];
    }
    return PlanetInfo("Unknown", "No information available", "", "", "");
}
