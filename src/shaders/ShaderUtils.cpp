#include "ShaderUtils.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::string ShaderUtils::readFile(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Basic shader sources
std::string ShaderUtils::getVertexShaderSource() {
    return readFile("shaders/shader.vert.glsl");
}

std::string ShaderUtils::getFragmentShaderSource() {
    return readFile("shaders/shader.frag.glsl");
}

// Selection shader sources
std::string ShaderUtils::getSelectionVertexShaderSource() {
    return readFile("shaders/selection.vert.glsl");
}

std::string ShaderUtils::getSelectionFragmentShaderSource() {
    return readFile("shaders/selection.frag.glsl");
}

// Skybox shader sources
std::string ShaderUtils::getSkyboxVertexShaderSource() {
    return readFile("shaders/skybox_vertex.glsl");
}

std::string ShaderUtils::getSkyboxFragmentShaderSource() {
    return readFile("shaders/skybox_fragment.glsl");
}

// Textured sphere shader sources
std::string ShaderUtils::getTexturedSphereVertexShaderSource() {
    return readFile("shaders/textured_sphere.vert.glsl");
}

std::string ShaderUtils::getTexturedSphereFragmentShaderSource() {
    return readFile("shaders/textured_sphere.frag.glsl");
}

// UI shader sources
std::string ShaderUtils::getUIVertexShaderSource() {
    return readFile("shaders/ui.vert.glsl");
}

std::string ShaderUtils::getUIFragmentShaderSource() {
    return readFile("shaders/ui.frag.glsl");
}

// Shader compilation methods
int ShaderUtils::compileVertexAndFragShaders() {
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderSourceStr = getVertexShaderSource();
    const char* vertexShaderSource = vertexShaderSourceStr.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderSourceStr = getFragmentShaderSource();
    const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

unsigned int ShaderUtils::compileSkyboxShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderStr = getSkyboxVertexShaderSource();
    const char* vertexShaderSource = vertexShaderStr.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderStr = getSkyboxFragmentShaderSource();
    const char* fragmentShaderSource = fragmentShaderStr.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint ShaderUtils::compileTexturedSphereShader() {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    std::string vsSourceStr = getTexturedSphereVertexShaderSource();
    const char* vsSource = vsSourceStr.c_str();
    glShaderSource(vs, 1, &vsSource, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fsSourceStr = getTexturedSphereFragmentShaderSource();
    const char* fsSource = fsSourceStr.c_str();
    glShaderSource(fs, 1, &fsSource, nullptr);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

GLuint ShaderUtils::compileUIShader() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderStr = getUIVertexShaderSource();
    const char* vertexShaderSource = vertexShaderStr.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderStr = getUIFragmentShaderSource();
    const char* fragmentShaderSource = fragmentShaderStr.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

ShaderPrograms ShaderUtils::setupShaderPrograms() {
    ShaderPrograms shaders;

    shaders.base = compileVertexAndFragShaders();
    glUseProgram(shaders.base);

    shaders.skybox = compileSkyboxShaderProgram();
    glUseProgram(shaders.skybox);
    glUniform1i(glGetUniformLocation(shaders.skybox, "skybox"), 0);

    shaders.orb = compileTexturedSphereShader();
    shaders.ui = compileUIShader();

    // Compile selection indicator shader
    int selectionVertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string selectionVertexSource = getSelectionVertexShaderSource();
    const char* selectionVertexSourcePtr = selectionVertexSource.c_str();
    glShaderSource(selectionVertexShader, 1, &selectionVertexSourcePtr, NULL);
    glCompileShader(selectionVertexShader);

    int selectionFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string selectionFragmentSource = getSelectionFragmentShaderSource();
    const char* selectionFragmentSourcePtr = selectionFragmentSource.c_str();
    glShaderSource(selectionFragmentShader, 1, &selectionFragmentSourcePtr, NULL);
    glCompileShader(selectionFragmentShader);

    shaders.selection = glCreateProgram();
    glAttachShader(shaders.selection, selectionVertexShader);
    glAttachShader(shaders.selection, selectionFragmentShader);
    glLinkProgram(shaders.selection);

    glDeleteShader(selectionVertexShader);
    glDeleteShader(selectionFragmentShader);

    return shaders;
}
