#pragma once
#include "CelestialBody.hpp"
#include <string>

class CelestialBodyFactory {
public:
    static CelestialBody createCelestialBody(const char* texturePath,
                                           float scale,
                                           float orbitRadius,
                                           float orbitSpeed,
                                           float rotationSpeed);
};
