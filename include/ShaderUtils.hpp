#pragma once
#include <string>
#include <GL/glew.h>
#include "ShaderPrograms.hpp"

class ShaderUtils {
public:
    static std::string readFile(const char* filePath);
    
    // Basic shader sources
    static std::string getVertexShaderSource();
    static std::string getFragmentShaderSource();
    
    // Selection shader sources
    static std::string getSelectionVertexShaderSource();
    static std::string getSelectionFragmentShaderSource();
    
    // Skybox shader sources
    static std::string getSkyboxVertexShaderSource();
    static std::string getSkyboxFragmentShaderSource();
    
    // Textured sphere shader sources
    static std::string getTexturedSphereVertexShaderSource();
    static std::string getTexturedSphereFragmentShaderSource();
    
    // UI shader sources
    static std::string getUIVertexShaderSource();
    static std::string getUIFragmentShaderSource();
    
    // Shader compilation methods
    static int compileVertexAndFragShaders();
    static unsigned int compileSkyboxShaderProgram();
    static GLuint compileTexturedSphereShader();
    static GLuint compileUIShader();
    
    // Setup all shader programs
    static ShaderPrograms setupShaderPrograms();
};
