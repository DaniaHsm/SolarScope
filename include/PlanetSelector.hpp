#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "CelestialBody.hpp"
#include "PlanetInfo.hpp"

class PlanetSelector {
public:
    std::vector<CelestialBody*> celestialBodies;
    std::vector<std::string> celestialNames;
    std::vector<PlanetInfo> planetInfos;
    int selectedIndex;
    bool was3Pressed;

    PlanetSelector();
    void addCelestialBody(CelestialBody* body, const std::string& name, const PlanetInfo& info);
    void nextSelection();
    CelestialBody* getSelectedBody();
    std::string getSelectedName();
    PlanetInfo getSelectedInfo();
    
    // Render selection indicator for the currently selected planet
    void renderSelectionIndicator(GLuint shader,
                                const glm::mat4& viewMatrix,
                                const glm::mat4& projectionMatrix) const;
                                
    // Setup planet selector with detailed information
    static PlanetSelector setupWithInfo(std::vector<CelestialBody*>& allBodies);
};
