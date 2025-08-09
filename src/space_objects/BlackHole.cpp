#include "include/space_objects/BlackHole.hpp"

BlackHole BlackHole::create() {
    BlackHole blackHole;
    blackHole.position = glm::vec3(0.0f, 0.0f, -20.0f); // Center position
    blackHole.strength = 0.0f; // Initial strength
    blackHole.active = false; // Not active initially
    blackHole.activationTime = 0.0f; // No activation time yet
    return blackHole;
}
