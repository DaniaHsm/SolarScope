#pragma once
#include <glm/glm.hpp>

struct TrailPoint {
    glm::vec3 position;
    float age;        // How old this trail point is (for fading)
    float brightness; // Brightness based on distance from sun
};
