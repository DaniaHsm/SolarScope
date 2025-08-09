#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

class SphereUtils {
public:
    static void generateSphereVerticesAndUVs(unsigned int rings,
                                           unsigned int sectors,
                                           std::vector<glm::vec3>& vertices,
                                           std::vector<glm::vec2>& uvs);
                                           
    static void generateSphereIndices(unsigned int rings,
                                    unsigned int sectors,
                                    std::vector<unsigned int>& indices);
                                    
    static GLuint setupSphereBuffers(const std::vector<glm::vec3>& vertices,
                                    const std::vector<glm::vec2>& uvs,
                                    const std::vector<unsigned int>& indices);
                                    
    static GLuint createTexturedSphereVAO(unsigned int rings,
                                         unsigned int sectors,
                                         unsigned int& indexCount);
};
