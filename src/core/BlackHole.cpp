#include "BlackHole.hpp"

BlackHole::BlackHole() : position(glm::vec3(0.0f)), strength(0.0f), active(false), activationTime(0.0f) {}

BlackHole createBlackHole(glm::vec3 center, const std::vector<CelestialBody*>& bodies) {
    BlackHole blackHole;
    blackHole.position = center;
    blackHole.strength = 0.0f;
    blackHole.active = false;
    blackHole.activationTime = 0.0f;

    // Store original positions and scales
    for (const auto& body : bodies) {
        blackHole.originalPositions.push_back(body->position);
        blackHole.originalScales.push_back(body->scale);
    }

    return blackHole;
}
