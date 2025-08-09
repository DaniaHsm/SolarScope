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
    std::vector<glm::vec3> resetPositions;     // OG positions on R
    std::vector<glm::vec3> resetScales;        // OG scales on R

    // Default constructor
    static BlackHole create();
};
