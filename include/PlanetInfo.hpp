#pragma once
#include <string>

struct PlanetInfo {
    std::string name;
    std::string description;
    std::string facts[3]; // Array of 3 key facts

    PlanetInfo() = default;
    PlanetInfo(const std::string& n, const std::string& desc, 
               const std::string& f1, const std::string& f2, const std::string& f3)
        : name(n), description(desc)
    {
        facts[0] = f1;
        facts[1] = f2;
        facts[2] = f3;
    }
};
