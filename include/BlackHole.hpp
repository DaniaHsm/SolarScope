#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "CelestialBody.hpp"

class BlackHole {
public:
    glm::vec3 position;
    float strength;
    bool active;
    float activationTime;
    std::vector<glm::vec3> originalPositions;  // Positions when X was pressed
    std::vector<glm::vec3> originalScales;     // Scales when X was pressed
    std::vector<glm::vec3> resetPositions;     // Orbital positions to return to when R is pressed
    std::vector<glm::vec3> resetScales;        // Original scales to return to when R is pressed

    BlackHole();
};

BlackHole createBlackHole(glm::vec3 center, const std::vector<CelestialBody*>& bodies);
