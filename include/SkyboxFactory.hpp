#pragma once

#include "Skybox.hpp"
#include <vector>
#include <string>

namespace SkyboxFactory {
    unsigned int loadCubemap(const std::vector<std::string>& faces);
    Skybox createSkybox(const std::vector<std::string>& faces);
}
