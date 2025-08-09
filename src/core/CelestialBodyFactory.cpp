#include "CelestialBodyFactory.hpp"
#include "SphereGeometry.hpp"
#include "TextureLoader.hpp"

CelestialBody CelestialBodyFactory::createCelestialBody(const char* texturePath,
                                                       float scale,
                                                       float orbitRadius,
                                                       float orbitSpeed,
                                                       float rotationSpeed) {
    CelestialBody body;
    body.scale = glm::vec3(scale);
    body.position = glm::vec3(0.0f);
    body.orbitRadius = orbitRadius;
    body.orbitSpeed = orbitSpeed;
    body.rotationSpeed = rotationSpeed;
    body.rotationAngle = 0.0f;

    body.vao = SphereGeometry::createTexturedSphereVAO(40, 40, body.indexCount);
    body.texture = TextureLoader::loadTexture(texturePath);

    return body;
}
