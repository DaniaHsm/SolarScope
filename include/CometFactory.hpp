#pragma once
#include "Comet.hpp"
#include <glm/glm.hpp>

class CometFactory {
public:
    static Comet createComet(const char* texturePath, 
                            const glm::vec3& orbitCenter, 
                            float semiMajorAxis, 
                            float eccentricity);
                            
    static void updateComet(Comet& comet, float dt, const glm::vec3& sunPosition);
    
    static void renderCometTrail(const Comet& comet, 
                                GLuint shader, 
                                const glm::mat4& viewMatrix, 
                                const glm::mat4& projectionMatrix);
};
