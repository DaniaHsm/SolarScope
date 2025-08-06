#include <GL/glew.h>		// OpenGL functions
#include <GLFW/glfw3.h>		// GLFW for window management
#include <fstream>
#include <glm/common.hpp>	// Math functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>
#define GLEW_STATIC 1
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assimp/Importer.hpp>	// 3D model management
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace glm;
using namespace std;

// Sets up GLFW window with OpenGL 3.2 core profile and mouse capture
GLFWwindow *initializeGLFW()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(
		800, 
		600, 
		"Comp371 - Project Assignment", 
		NULL, 
		NULL
	);

    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    return window;
}

// Initializes GLEW for OpenGL function loading
bool initializeOpenGL()
{
    glewExperimental = true;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }
    return true;
}

// Container for different shader programs used in the scene
// - base: For 3D model rendering (duck)
// - skybox: For space background rendering
// - orb: For celestial body rendering with special lighting (planets)
struct ShaderPrograms
{
    int base;
    unsigned int skybox;
    unsigned int orb;
};

// Helper function to read .glsl files
std::string readFile(const char *filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Shader source loaders
std::string getVertexShaderSource()
{
    return readFile("shaders/shader.vert.glsl");
}

std::string getFragmentShaderSource()
{
    return readFile("shaders/shader.frag.glsl");
}

int compileVertexAndFragShaders()
{

    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderSourceStr = getVertexShaderSource();
    const char *vertexShaderSource = vertexShaderSourceStr.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderSourceStr = getFragmentShaderSource();
    const char *fragmentShaderSource = fragmentShaderSourceStr.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Loads vertex shader specific to skybox rendering
std::string getSkyboxVertexShaderSource()
{
    return readFile("shaders/skybox_vertex.glsl");
}

// Loads fragment shader specific to skybox rendering
std::string getSkyboxFragmentShaderSource()
{
    return readFile("shaders/skybox_fragment.glsl");
}

unsigned int compileSkyboxShaderProgram()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    std::string vertexShaderStr = getSkyboxVertexShaderSource();
    const char *vertexShaderSource = vertexShaderStr.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fragmentShaderStr = getSkyboxFragmentShaderSource();
    const char *fragmentShaderSource = fragmentShaderStr.c_str();
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

// Loads vertex shader for celestial bodies (sun, planets)
std::string getTexturedSphereVertexShaderSource()
{
    return readFile("shaders/textured_sphere.vert.glsl");
}

// Loads fragment shader for celestial bodies with texture support
std::string getTexturedSphereFragmentShaderSource()
{
    return readFile("shaders/textured_sphere.frag.glsl");
}

// Compiles and links the shader program for celestial bodies
// Handles both vertex and fragment shaders for textured spheres
GLuint compileTexturedSphereShader()
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    std::string vsSourceStr = getTexturedSphereVertexShaderSource();
    const char *vsSource = vsSourceStr.c_str();
    glShaderSource(vs, 1, &vsSource, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    std::string fsSourceStr = getTexturedSphereFragmentShaderSource();
    const char *fsSource = fsSourceStr.c_str();
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

ShaderPrograms setupShaderPrograms()
{
    ShaderPrograms shaders;

    shaders.base = compileVertexAndFragShaders();
    glUseProgram(shaders.base);

    shaders.skybox = compileSkyboxShaderProgram();
    glUseProgram(shaders.skybox);
    glUniform1i(glGetUniformLocation(shaders.skybox, "skybox"), 0);

    shaders.orb = compileTexturedSphereShader();

    return shaders;
}

// Manages camera movement and view:
// - Supports both first-person and third-person views
// - Handles position, orientation, and movement speeds
struct Camera
{
    vec3 position;       	// Camera position in 3D space
    vec3 lookAt;         	// Direction the camera is facing
    vec3 up;             	// Up vector for camera orientation
    float speed;         	// Normal movement speed
    float fastSpeed;    	// Sprint movement speed
    float horizontalAngle;  // Left/right rotation
    float verticalAngle;    // Up/down rotation
    bool firstPerson;   	// Toggle between first/third person
};

// Initializes camera with default settings:
// - Starting position above and behind origin
// - Looking forward with standard up vector
// - Default movement and rotation speeds
// - First-person view mode enabled
Camera setupCamera()
{
    Camera camera;
    camera.position = vec3(0.0f, 2.0f, 5.0f);
    camera.lookAt = vec3(0.0f, 0.0f, -1.0f);
    camera.up = vec3(0.0f, 1.0f, 0.0f);
    camera.speed = 3.0f;
    camera.fastSpeed = 6.0f;
    camera.horizontalAngle = 90.0f;
    camera.verticalAngle = 0.0f;
    camera.firstPerson = true;
    return camera;
}

// Updates the view matrix based on camera mode and position
// Handles both first-person and third-person perspectives:
// - First-person: Direct view from camera position
// - Third-person: Orbital view around target with fixed radius
mat4 updateViewMatrix(const Camera &camera)
{
    if (camera.firstPerson)
    {
        return lookAt(camera.position, camera.position + camera.lookAt, camera.up);
    }
    else
    {
        // Distance from target in third-person
        float radius = 1.5f;
        glm::vec3 position = camera.position - radius * camera.lookAt;
        return lookAt(position, position + camera.lookAt, camera.up);
    }
}

// Represents a celestial body (sun, planet, or moon):
// - Handles rendering properties (VAO, texture, indices)
// - Controls position, scale, and rotation
// - Manages orbital movement parameters
struct CelestialBody
{
    GLuint vao;          		// Vertex Array Object for the sphere
    GLuint texture;      		// Body's surface texture
    unsigned int indexCount;  	// Number of indices for rendering
    vec3 position;       		// Current position in space
    vec3 scale;          		// Size of the celestial body
    float rotationAngle; 		// Current rotation around its axis
    float rotationSpeed; 		// Speed of rotation around its axis
    float orbitRadius;   		// Distance from the center of orbit
    float orbitSpeed;    		// Speed of orbital movement
};

// Planet Selection System
struct PlanetSelector {
    vector<CelestialBody*> celestialBodies;
    vector<string> celestialNames;
    int selectedIndex;
    bool was3Pressed;
    
    PlanetSelector() : selectedIndex(0), was3Pressed(false) {}
    
    void addCelestialBody(CelestialBody* body, const string& name) {
        celestialBodies.push_back(body);
        celestialNames.push_back(name);
    }
    
    void nextSelection() {
        selectedIndex = (selectedIndex + 1) % celestialBodies.size();
        cout << "Selected: " << celestialNames[selectedIndex] << endl;
    }
    
    CelestialBody* getSelectedBody() {
        if (selectedIndex < celestialBodies.size()) {
            return celestialBodies[selectedIndex];
        }
        return nullptr;
    }
    
    string getSelectedName() {
        if (selectedIndex < celestialNames.size()) {
            return celestialNames[selectedIndex];
        }
        return "Unknown";
    }
};

// Black hole structure with proper functionality
struct BlackHole {
    vec3 position;
    float strength;
    bool active;
    float activationTime;
    vector<vec3> originalPositions;  // Positions when X was pressed (for black hole effect)
    vector<vec3> originalScales;     // Scales when X was pressed (for black hole effect)
    vector<vec3> resetPositions;     // Orbital positions to return to when R is pressed
    vector<vec3> resetScales;        // Original scales to return to when R is pressed
    
    // Constructor
    BlackHole() : position(vec3(0.0f)), strength(0.0f), active(false), activationTime(0.0f) {}
};

// Function to initialize black hole with original positions
BlackHole createBlackHole(vec3 center, const vector<CelestialBody*>& bodies) {
    BlackHole blackHole;
    blackHole.position = center;
    blackHole.strength = 0.0f;
    blackHole.active = false;
    blackHole.activationTime = 0.0f;
    
    // Store original positions and scales
    for (const auto& body : bodies) {
        blackHole.originalPositions.push_back(body->position);
        blackHole.originalScales.push_back(body->scale);
    }
    
    return blackHole;
}

// Saturn's rings structure
struct PlanetRing
{
    GLuint vao;
    GLuint texture;
    unsigned int indexCount;
    float innerRadius;
    float outerRadius;
};

// Generates vertices and texture coordinates for a UV sphere
// Creates a sphere by dividing it into rings and sectors:
// - rings: vertical divisions from pole to pole
// - sectors: horizontal divisions around the sphere
// - Generates proper UV mapping for textures
void generateSphereVerticesAndUVs(
	unsigned int rings,
    unsigned int sectors,
    std::vector<vec3> &vertices,
	std::vector<vec2> &uvs)
{
    float const R = 1.0f / float(rings - 1);    // Ring step
    float const S = 1.0f / float(sectors - 1);  // Sector step

    for (unsigned int r = 0; r < rings; ++r)
    {
        for (unsigned int s = 0; s < sectors; ++s)
        {
            float const phi = -glm::half_pi<float>() + glm::pi<float>() * r * R;
            float const theta = 2 * glm::pi<float>() * s * S;

            float const y = sin(phi);
            float const x = cos(theta) * sin(glm::pi<float>() * r * R);
            float const z = sin(theta) * sin(glm::pi<float>() * r * R);

            vertices.push_back(vec3(x, y, z));
            uvs.push_back(vec2(s * S, r * R));
        }
    }
}

// Generates triangle indices for sphere mesh
// Creates triangles between adjacent rings and sectors:
// - Builds triangles in counter-clockwise order
// - Ensures proper face winding for correct rendering
void generateSphereIndices(unsigned int rings, 
	unsigned int sectors, 
	std::vector<unsigned int> &indices)
{
    for (unsigned int r = 0; r < rings - 1; ++r)
    {
        for (unsigned int s = 0; s < sectors - 1; ++s)
        {
            unsigned int current = r * sectors + s;
            unsigned int next = current + 1;
            unsigned int below = (r + 1) * sectors + s;
            unsigned int belowNext = below + 1;

            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(belowNext);

            indices.push_back(current);
            indices.push_back(belowNext);
            indices.push_back(below);
        }
    }
}

// Sets up OpenGL buffers for sphere rendering
// Creates and configures:
// - Vertex Array Object (VAO)
// - Vertex Buffer Objects (VBOs) for positions and UVs
// - Element Buffer Object (EBO) for indices
GLuint setupSphereBuffers(
	const std::vector<vec3> &vertices,
    const std::vector<vec2> &uvs,
    const std::vector<unsigned int> &indices)
{
    GLuint vao, vbo[2], ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(2, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, 
		vertices.size() * sizeof(vec3), 
		&vertices[0], GL_STATIC_DRAW
	);
	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
		GL_ELEMENT_ARRAY_BUFFER, 
		indices.size() * sizeof(unsigned int), 
		&indices[0], GL_STATIC_DRAW
	);

    return vao;
}

GLuint createTexturedSphereVAO(
	unsigned int rings, 
	unsigned int sectors, 
	unsigned int &indexCount)
{
    std::vector<vec3> vertices;
    std::vector<vec2> uvs;
    std::vector<unsigned int> indices;

    generateSphereVerticesAndUVs(rings, sectors, vertices, uvs);
    generateSphereIndices(rings, sectors, indices);

    indexCount = indices.size();
    return setupSphereBuffers(vertices, uvs, indices);
}

GLuint loadTexture(const char *path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    std::cout << "Loading texture from path: " << path << std::endl;
    
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << "Texture loaded successfully: " << width << "x" << height << " channels: " << nrChannels << std::endl;
        
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Check for OpenGL errors
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error in loadTexture: " << err << std::endl;
        }
        
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        stbi_image_free(data);
        return 0;  // Return 0 to indicate failure
    }
    return textureID;
}

CelestialBody createCelestialBody(
	const char *texturePath,
    float scale,
    float orbitRadius,
    float orbitSpeed,
	float rotationSpeed)
{
    CelestialBody body;
    body.scale = vec3(scale);
    body.position = vec3(0.0f);
    body.orbitRadius = orbitRadius;
    body.orbitSpeed = orbitSpeed;
    body.rotationSpeed = rotationSpeed;
    body.rotationAngle = 0.0f;

    body.vao = createTexturedSphereVAO(40, 40, body.indexCount);
    body.texture = loadTexture(texturePath);

    return body;
}

// Add these structures right after your CelestialBody struct

// Represents a point in the comet's trail
struct TrailPoint {
    vec3 position;
    float age;        // How old this trail point is (for fading)
    float brightness; // Brightness based on distance from sun
};

// Comet with elliptical orbit and particle trail
struct Comet {
    CelestialBody body;           // Reuse existing celestial body for the head
    std::vector<TrailPoint> trail; // Trail points
    float orbitAngle;             // Current angle in elliptical orbit
    float eccentricity;           // How elliptical the orbit is (0 = circle, 0.9 = very elliptical)
    float semiMajorAxis;          // Size of the orbit
    vec3 orbitCenter;             // Center point of orbit
    GLuint trailVAO;              // VAO for trail rendering
    GLuint trailVBO;              // VBO for trail vertices
    int maxTrailPoints;           // Maximum trail length
    float lastTrailUpdate;        // Time tracking for trail updates
    
    void updateTrail(float currentTime, const vec3& sunPosition) {
        // Add new trail point every 0.1 seconds
        if (currentTime - lastTrailUpdate > 0.1f) {
            TrailPoint newPoint;
            newPoint.position = body.position;
            newPoint.age = 0.0f;
            
            // Brightness based on distance from sun (closer = brighter trail)
            float distanceFromSun = length(body.position - sunPosition);
            newPoint.brightness = 1.0f / (1.0f + distanceFromSun * 0.1f);
            
            trail.insert(trail.begin(), newPoint);
            
            // Remove old trail points
            if (trail.size() > maxTrailPoints) {
                trail.pop_back();
            }
            
            lastTrailUpdate = currentTime;
        }
        
        // Age all trail points
        for (auto& point : trail) {
            point.age += 0.016f; // Approximate 60fps
        }
        
        updateTrailVBO();
    }
    
    void updateTrailVBO() {
        if (trail.empty()) return;
        
        std::vector<vec3> vertices;
        std::vector<vec3> colors;
        
        // Create line segments for the trail
        for (size_t i = 0; i < trail.size(); ++i) {
            vertices.push_back(trail[i].position);
            
            // Color fades from bright blue/white to dark blue based on age
            float fade = 1.0f - (trail[i].age / 10.0f); // Fade over 10 seconds
            fade = std::max(0.0f, fade);
            
            // Comet tail color - blue/white mix
            vec3 color = vec3(0.7f + 0.3f * trail[i].brightness, 
                             0.8f + 0.2f * trail[i].brightness, 
                             1.0f) * fade;
            colors.push_back(color);
        }
        
        glBindVertexArray(trailVAO);
        glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
        glBufferData(GL_ARRAY_BUFFER, 
                    vertices.size() * sizeof(vec3) + colors.size() * sizeof(vec3), 
                    nullptr, GL_DYNAMIC_DRAW);
        
        // Upload vertices
        glBufferSubData(GL_ARRAY_BUFFER, 0, 
                       vertices.size() * sizeof(vec3), &vertices[0]);
        
        // Upload colors
        glBufferSubData(GL_ARRAY_BUFFER, 
                       vertices.size() * sizeof(vec3),
                       colors.size() * sizeof(vec3), &colors[0]);
        
        // Set up vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 
                             (void*)(vertices.size() * sizeof(vec3)));
        glEnableVertexAttribArray(1);
    }
};

// Function to create a comet
Comet createComet(const char* texturePath, const vec3& orbitCenter, 
                  float semiMajorAxis, float eccentricity) {
    Comet comet;
    
    // Create the comet head using existing celestial body system
    comet.body = createCelestialBody(texturePath, 0.05f, 0.0f, 0.0f, 10.0f);
    
    // Set up orbital parameters
    comet.orbitCenter = orbitCenter;
    comet.semiMajorAxis = semiMajorAxis;
    comet.eccentricity = eccentricity;
    comet.orbitAngle = 0.0f;
    comet.maxTrailPoints = 150; // Long, visible trail
    comet.lastTrailUpdate = 0.0f;
    
    // Set up trail rendering
    glGenVertexArrays(1, &comet.trailVAO);
    glGenBuffers(1, &comet.trailVBO);
    
    return comet;
}

// Function to update comet position in elliptical orbit
void updateComet(Comet& comet, float dt, const vec3& sunPosition) {
    // Update orbital position
    comet.orbitAngle += 0.5f * dt; // Slow orbital speed
    
    // Calculate elliptical orbit position
    float a = comet.semiMajorAxis; // Semi-major axis
    float e = comet.eccentricity;  // Eccentricity
    
    // Elliptical orbit math
    float r = a * (1.0f - e * e) / (1.0f + e * cos(comet.orbitAngle));
    float x = r * cos(comet.orbitAngle);
    float z = r * sin(comet.orbitAngle);
    
    comet.body.position = comet.orbitCenter + vec3(x, 0.0f, z);
    
    // Update rotation
    comet.body.rotationAngle += comet.body.rotationSpeed * dt;
    
    // Update trail
    comet.updateTrail(glfwGetTime(), sunPosition);
}

// Function to render comet trail
void renderCometTrail(const Comet& comet, GLuint shader, 
                     const mat4& viewMatrix, const mat4& projectionMatrix) {
    if (comet.trail.size() < 2) return;
    
    glUseProgram(shader);
    glBindVertexArray(comet.trailVAO);
    
    // Set matrices
    mat4 worldMatrix = mat4(1.0f); // Identity - trail points are in world space
    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
    
    // Enable blending for trail transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Draw trail as line strip
    glDrawArrays(GL_LINE_STRIP, 0, comet.trail.size());
    
    glDisable(GL_BLEND);
}


// Function to create Saturn's rings
PlanetRing createSaturnRings()
{
    PlanetRing ring;
    
    // Create ring geometry (simplified as a flat disk with hole)
    std::vector<vec3> vertices;
    std::vector<vec2> uvs;
    std::vector<unsigned int> indices;
    
    float innerRadius = 1.2f;  // Inner edge of rings
    float outerRadius = 2.0f;  // Outer edge of rings
    int segments = 64;         // Number of segments around the ring
    
    // Generate ring vertices
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (float)i / segments * 2.0f * 3.14159f;
        float cosA = cos(angle);
        float sinA = sin(angle);
        
        // Inner vertex
        vertices.push_back(vec3(innerRadius * cosA, 0.0f, innerRadius * sinA));
        uvs.push_back(vec2(0.0f, (float)i / segments));
        
        // Outer vertex
        vertices.push_back(vec3(outerRadius * cosA, 0.0f, outerRadius * sinA));
        uvs.push_back(vec2(1.0f, (float)i / segments));
    }
    
    // Generate indices for ring triangles
    for (int i = 0; i < segments * 2; i += 2)
    {
        // First triangle
        indices.push_back(i);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
        
        // Second triangle
        indices.push_back(i + 1);
        indices.push_back(i + 3);
        indices.push_back(i + 2);
    }
    
    ring.vao = setupSphereBuffers(vertices, uvs, indices);
    ring.indexCount = indices.size();
    ring.texture = loadTexture("textures/saturn_rings.png"); // Semi-transparent ring texture
    ring.innerRadius = innerRadius;
    ring.outerRadius = outerRadius;
    
    return ring;
}

// Manages the space background environment:
// - Creates a skybox cube with 6 textured faces
// - Handles cubemap texture loading
// - Renders the environment around the scene
struct Skybox
{
    GLuint vao;     // Vertex Array Object for the cube
    GLuint vbo;     // Vertex Buffer Object
    GLuint texture; // Texture ID
};

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(
			faces[i].c_str(), 
			&width, 
			&height, 
			&nrChannels, 
			0
		);

        if (data)
        {
            glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
       			0, 
                GL_RGB, 
		    	width, 
		    	height, 
		    	0, 
		    	GL_RGB, 
		    	GL_UNSIGNED_BYTE, 
		    	data 
			);
			stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

Skybox createSkybox(const std::vector<std::string> &faces)
{
    Skybox skybox;

    float vertices[] = {
		-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
        -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

        1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
        1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f
	};

    glGenVertexArrays(1, &skybox.vao);
    glGenBuffers(1, &skybox.vbo);

    glBindVertexArray(skybox.vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    skybox.texture = loadCubemap(faces);
    return skybox;
}

void renderSkybox(
	const Skybox &skybox, 
	GLuint shader, 
	const mat4 &viewMatrix, 
	const mat4 &projectionMatrix)
{
    glDepthFunc(GL_LEQUAL);
    glUseProgram(shader);

    mat4 skyboxView = mat4(mat3(viewMatrix));

    glUniformMatrix4fv(
		glGetUniformLocation(shader, "view"), 
		1, 
		GL_FALSE, 
		&skyboxView[0][0]
	);

    glUniformMatrix4fv(
		glGetUniformLocation(shader, "projection"), 
		1, 
		GL_FALSE, 
		&projectionMatrix[0][0]
	);

    glBindVertexArray(skybox.vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.texture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
}

int createVertexBufferObject()
{
    vec3 vertexArray[] = {
		vec3(-0.5f, -0.5f, -0.5f), vec3(1.0f, 0.0f, 0.0f), 
		vec3(-0.5f, -0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), 
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), 

        vec3(-0.5f, -0.5f, -0.5f), vec3(1.0f, 0.0f, 0.0f), 
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f), 
		vec3(-0.5f, 0.5f, -0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f), 
		vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f), 
		vec3(-0.5f, 0.5f, -0.5f),  vec3(0.0f, 0.0f, 1.0f),

        vec3(0.5f, 0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f), 
		vec3(0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f), 
		vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 0.0f, 1.0f),

        vec3(0.5f, -0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f), 
		vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 1.0f, 1.0f), 
		vec3(0.5f, -0.5f, -0.5f), vec3(0.0f, 1.0f, 1.0f),

        vec3(0.5f, -0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f), 
		vec3(-0.5f, -0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f), 
		vec3(-0.5f, -0.5f, -0.5f), vec3(0.0f, 1.0f, 1.0f),

        vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), 
		vec3(-0.5f, -0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), 
		vec3(0.5f, -0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), 
		vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), 
		vec3(0.5f, -0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f), 
		vec3(0.5f, -0.5f, -0.5f), vec3(1.0f, 0.0f, 1.0f), 
		vec3(0.5f, 0.5f, -0.5f), vec3(1.0f, 0.0f, 1.0f),

        vec3(0.5f, -0.5f, -0.5f), vec3(1.0f, 0.0f, 1.0f), 
		vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f), 
		vec3(0.5f, -0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f), 
		vec3(0.5f, 0.5f, -0.5f), vec3(1.0f, 1.0f, 0.0f), 
		vec3(-0.5f, 0.5f, -0.5f), vec3(1.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f), 
		vec3(-0.5f, 0.5f, -0.5f), vec3(1.0f, 1.0f, 0.0f), 
		vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f)
	};

    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);

    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(
		0, 
		3, 
		GL_FLOAT, 
		GL_FALSE, 
		2 * sizeof(vec3), 
		(void *)0
	);
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(
		1, 
		3, 
		GL_FLOAT, 
		GL_FALSE, 
		2 * sizeof(vec3), 
		(void *)sizeof(vec3)
	);
    glEnableVertexAttribArray(1);


    return vertexBufferObject;
}

void updateCelestialBody(
	CelestialBody &body, 
	const vec3 &centerPosition, 
	float baseAngle, 
	float dt)
{
    body.rotationAngle += body.rotationSpeed * dt;

    float orbitAngle = baseAngle * body.orbitSpeed;
    body.position = centerPosition + vec3(body.orbitRadius * cos(radians(orbitAngle)), 
	0.0f, body.orbitRadius * sin(radians(orbitAngle)));
}

mat4 getCelestialBodyMatrix(const CelestialBody &body)
{
    return translate(
		mat4(1.0f), 
		body.position
	) 
	* rotate(
		mat4(1.0f), 
		radians(body.rotationAngle), 
		vec3(0.0f, 1.0f, 0.0f)
	)
    * rotate(
        mat4(1.0f),
        radians(180.0f),
        vec3(1.0f, 0.0f, 0.0f)
    )
	* scale(
		mat4(1.0f), 
		body.scale
	);
}

void renderCelestialBody(
    const CelestialBody &body,
    GLuint shader,
    const mat4 &viewMatrix,
    const mat4 &projectionMatrix,
    const vec3 &lightPos,
    const vec3 &viewPos,
    bool isSun = false,
    const vector<vec3> &allPlanetPositions = vector<vec3>(),
    const vector<float> &allPlanetRadii = vector<float>())
{
    // Disable culling for celestial bodies to ensure correct appearance
    glDisable(GL_CULL_FACE);
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, body.texture);
    glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader, "isSun"), isSun ? 1 : 0);

    // Pass planet data for shadow calculations
    if (!isSun && !allPlanetPositions.empty()) {
        glUniform3fv(glGetUniformLocation(shader, "planetPositions"), allPlanetPositions.size(), &allPlanetPositions[0][0]);
        glUniform1fv(glGetUniformLocation(shader, "planetRadii"), allPlanetRadii.size(), &allPlanetRadii[0]);
        glUniform1i(glGetUniformLocation(shader, "numPlanets"), allPlanetPositions.size());
    }

    mat4 worldMatrix = getCelestialBodyMatrix(body);

    glUniformMatrix4fv(
		glGetUniformLocation(shader, "projectionMatrix"), 
		1, 
		GL_FALSE, 
		&projectionMatrix[0][0]
	);

    glUniformMatrix4fv(
		glGetUniformLocation(shader, "viewMatrix"), 
		1, 
		GL_FALSE, 
		&viewMatrix[0][0]
	);

    glUniformMatrix4fv(
		glGetUniformLocation(shader, "worldMatrix"), 
		1, 
		GL_FALSE, 
		&worldMatrix[0][0]
	);

    // Use a fixed view direction for lighting calculations
    vec3 fixedViewPos = vec3(0.0f, 0.0f, 0.0f);  // Origin point
    
    glUniform3fv(
		glGetUniformLocation(shader, "lightPos"), 
		1, 
		&lightPos[0]
	);

    glUniform3fv(
    glGetUniformLocation(shader, "viewPos"), 
    1, 
    &viewPos[0]  // Use the actual camera position passed to the function
    );

    glBindVertexArray(body.vao);
    glDrawElements(GL_TRIANGLES, body.indexCount, GL_UNSIGNED_INT, 0);
	
	// Re-enable culling after rendering celestial bodies
	glEnable(GL_CULL_FACE);
}

// Function to render Saturn's rings
void renderPlanetRings(const PlanetRing& ring, const CelestialBody& planet,
                      GLuint shader, const mat4& viewMatrix, const mat4& projectionMatrix,
                      const vec3& lightPos, const vec3& viewPos)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);  // Rings should be visible from both sides
    
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ring.texture);
    glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader, "isSun"), 0); // Rings are not the sun
    
    // Position rings at planet location
    mat4 worldMatrix = translate(mat4(1.0f), planet.position) 
                     * rotate(mat4(1.0f), radians(-10.0f), vec3(1.0f, 0.0f, 0.0f)) // Tilt rings
                     * scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));
    
    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
    
    // Set lighting uniforms
    glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, &viewPos[0]);
    
    glBindVertexArray(ring.vao);
    glDrawElements(GL_TRIANGLES, ring.indexCount, GL_UNSIGNED_INT, 0);
    
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
}

// Camera update function for planet selection mode
void updateCameraForSelectedPlanet(Camera& camera, CelestialBody* selectedBody, float dt) {
    if (!selectedBody) return;
    
    // Position camera at a good viewing distance from the selected planet
    float viewingDistance = selectedBody->scale.x * 8.0f; // Adjust multiplier as needed
    viewingDistance = std::max(3.0f, viewingDistance);// Minimum distance

    
    // Calculate desired camera position (slightly above and behind the planet)
    vec3 targetPosition = selectedBody->position + vec3(0.0f, viewingDistance * 0.3f, viewingDistance);
    
    // Smooth camera movement (lerp towards target)
    float lerpSpeed = 2.0f * dt; // Adjust speed as needed
    camera.position = mix(camera.position, targetPosition, lerpSpeed);
    
    // Make camera look at the selected planet
    vec3 directionToPlanet = normalize(selectedBody->position - camera.position);
    camera.lookAt = directionToPlanet;
}

// Add visual selection indicator (simple wireframe sphere around selected planet)
void renderSelectionIndicator(CelestialBody* selectedBody, GLuint shader, 
                            const mat4& viewMatrix, const mat4& projectionMatrix) {
    if (!selectedBody) return;
    
    glUseProgram(shader);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
    glLineWidth(3.0f); // Thick lines
    
    // Create a slightly larger sphere around the selected planet
    float indicatorScale = selectedBody->scale.x * 1.5f;
    mat4 worldMatrix = translate(mat4(1.0f), selectedBody->position) * 
                      scale(mat4(1.0f), vec3(indicatorScale));
    
    glUniformMatrix4fv(glGetUniformLocation(shader, "worldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projectionMatrix"), 1, GL_FALSE, &projectionMatrix[0][0]);
    
    // Set a bright color for the selection indicator
    vec3 selectionColor = vec3(1.0f, 1.0f, 0.0f); // Bright yellow
    glUniform3fv(glGetUniformLocation(shader, "selectionColor"), 1, &selectionColor[0]);
    
    // Render the wireframe sphere (you can use the same VAO as the planet)
    glBindVertexArray(selectedBody->vao);
    glDrawElements(GL_TRIANGLES, selectedBody->indexCount, GL_UNSIGNED_INT, 0);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Back to solid mode
    glLineWidth(1.0f); // Reset line width
}

void updateCameraAngles(Camera &camera, float dx, float dy, float dt)
{
    const float cameraAngularSpeed = 60.0f;
    camera.horizontalAngle -= dx * cameraAngularSpeed * dt;
    camera.verticalAngle -= dy * cameraAngularSpeed * dt;

    camera.verticalAngle = std::max(-85.0f, std::min(85.0f, camera.verticalAngle));

    float theta = radians(camera.horizontalAngle);
    float phi = radians(camera.verticalAngle);
    camera.lookAt = vec3(cos(phi) * cos(theta), sin(phi), -cos(phi) * sin(theta));
}

void updateCameraPosition(Camera &camera, GLFWwindow *window, float dt)
{
    bool fastCam = 
		glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || 
		glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

    float currentCameraSpeed = fastCam ? camera.fastSpeed : camera.speed;

    vec3 cameraSideVector = glm::cross(camera.lookAt, camera.up);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.position += camera.lookAt * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.position -= camera.lookAt * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.position -= cameraSideVector * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.position += cameraSideVector * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.position += camera.up * dt * currentCameraSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.position -= camera.up * dt * currentCameraSpeed;
    }
}

// Represents a single mesh component of a 3D model:
// - Stores geometry data (vertices, normals, UVs)
// - Manages OpenGL buffers and attributes
// - Handles texture mapping
struct Mesh
{
    std::vector<vec3> vertices;    		// 3D vertex positions
    std::vector<vec3> normals;     		// Vertex normals for lighting
    std::vector<vec2> texCoords;   		// Texture coordinates
    std::vector<unsigned int> indices;  // Vertex indices for drawing
    GLuint VAO;                    		// Vertex Array Object
    GLuint texture;                		// Diffuse texture

    void setupMesh()
    {
        GLuint VBO, normalVBO, texVBO, EBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &normalVBO);
        glGenBuffers(1, &texVBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // Vertex positions
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Normals
        if (!normals.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }

        // Texture coordinates
        if (!texCoords.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, texVBO);
            glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(vec2), &texCoords[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        }

        // Indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glBindVertexArray(0);
    }
};

// Container for complete 3D model (duck):
// - Manages collection of meshes
// - Handles model loading via Assimp
// - Controls model rendering with error checking
struct Model
{
    std::vector<Mesh> meshes;

    void Draw(GLuint shader)
    {
        for (const auto &mesh : meshes)
        {
            // Bind texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.texture);
            GLint texLocation = glGetUniformLocation(shader, "texture1");
            if (texLocation == -1) {
                std::cerr << "Warning: Uniform 'texture1' not found in shader" << std::endl;
            }
            glUniform1i(texLocation, 0);

            // Draw mesh
            glBindVertexArray(mesh.VAO);
            if (mesh.indices.empty()) {
                std::cerr << "Warning: Mesh has no indices" << std::endl;
            } else {
                glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
            }
            glBindVertexArray(0);
            
            // Check for OpenGL errors
            GLenum err;
            while ((err = glGetError()) != GL_NO_ERROR) {
                std::cerr << "OpenGL error in Draw: " << err << std::endl;
            }
        }
    }
};

void processNode(Model &model, aiNode *node, const aiScene *scene) {
    // Process all meshes in this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        if (!mesh) {
            std::cerr << "Warning: Null mesh encountered" << std::endl;
            continue;
        }

        std::cout << "Processing mesh with " << mesh->mNumVertices << " vertices" << std::endl;

        Mesh newMesh;
        std::cout << "Processing mesh with " << mesh->mNumVertices << " vertices" << std::endl;
        
        // Load material/texture first
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            if (material) {
                aiString texturePath;
                if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
                    std::string fullPath = std::string("models/rubber_duck/textures/material_baseColor.jpeg");
                    std::cout << "Loading texture from: " << fullPath << std::endl;
                    newMesh.texture = loadTexture(fullPath.c_str());
                    if (newMesh.texture == 0) {
                        std::cerr << "Failed to load texture!" << std::endl;
                    } else {
                        std::cout << "Texture loaded successfully with ID: " << newMesh.texture << std::endl;
                    }
                }
            }
        }

        // Process vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            newMesh.vertices.push_back(vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            ));

            if (mesh->HasNormals()) {
                newMesh.normals.push_back(vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                ));
            }

            if (mesh->mTextureCoords[0]) {
                newMesh.texCoords.push_back(vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                ));
            } else {
                newMesh.texCoords.push_back(vec2(0.0f, 0.0f));
            }
        }

        // Process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                newMesh.indices.push_back(face.mIndices[j]);
            }
        }

        if (!newMesh.vertices.empty() && !newMesh.indices.empty()) {
            newMesh.setupMesh();
            model.meshes.push_back(newMesh);
            std::cout << "Added mesh with " << newMesh.vertices.size() << " vertices and " 
                     << newMesh.indices.size() << " indices" << std::endl;
        }
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(model, node->mChildren[i], scene);
    }
}

// Loads a 3D model from file using Assimp
// - Supports multiple mesh formats (GLTF, OBJ, etc.)
// - Processes the model for OpenGL rendering
// - Handles textures, normals, and vertex data
// - Returns a complete model ready for rendering
Model loadModel(const char *path)
{
    Model model;
    Assimp::Importer importer;
    std::cout << "Attempting to load model from: " << path << std::endl;
    
    const aiScene *scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenNormals | 
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ValidateDataStructure |
        aiProcess_PreTransformVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return model;
    }

    std::cout << "Model loaded successfully. Number of meshes: " << scene->mNumMeshes << std::endl;
    std::cout << "Number of materials: " << scene->mNumMaterials << std::endl;
    std::cout << "Number of children in root node: " << scene->mRootNode->mNumChildren << std::endl;

    // Process all nodes recursively starting from the root
    processNode(model, scene->mRootNode, scene);

    return model;
}

int main(int argc, char *argv[])
{
    // Initialize GLFW and OpenGL
    GLFWwindow *window = initializeGLFW();
    if (!window)
    {
        return -1;
    }

    if (!initializeOpenGL())
    {
        glfwTerminate();
        return -1;
    }

    // Enable OpenGL features
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Setup shaders and camera
    ShaderPrograms shaders = setupShaderPrograms();
    Camera camera = setupCamera();

    // Setup projection and view matrices
    mat4 projectionMatrix = glm::perspective(
		70.0f, 
		800.0f / 600.0f, 
		0.01f, 
		100.0f
	);

    mat4 viewMatrix = updateViewMatrix(camera);

    GLuint projectionMatrixLocation = glGetUniformLocation(
		shaders.base, 
		"projectionMatrix"
	);

    GLuint viewMatrixLocation = glGetUniformLocation(
		shaders.base, 
		"viewMatrix"
	);

    glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);
    glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

    // Create scene objects
    int vao = createVertexBufferObject();
    Model duckModel = loadModel("models/rubber_duck/scene.gltf");

    // Setup celestial bodies with realistic proportions
    // Using a scale where Earth = 0.3f as base reference

    /// SUN - Center of the system
    CelestialBody sun = createCelestialBody(
        "textures/sun.jpg", 
        4.0f,      // Large but viewable size
        0.0f,      // No orbit - center of system
        0.0f,      // No orbital movement
        15.0f      // Rotation speed
    );
    sun.position = vec3(0.0f, 0.0f, -20.0f);

    // MERCURY - Smallest planet, closest orbit
    CelestialBody mercury = createCelestialBody(
        "textures/mercury.jpg",
        0.11f,     // Small size
        8.0f,      // Safe distance from sun (was 3.0f)
        2.0f,      // Fastest orbital speed
        35.0f      // Fast rotation
    );

    // VENUS - Second planet
    CelestialBody venus = createCelestialBody(
        "textures/venus.jpg",
        0.28f,     // Venus size
        10.0f,     // Safe distance from mercury (was 4.5f)
        1.6f,      // Orbital speed
        -12.0f     // Slow retrograde rotation
    );

    // EARTH - Third planet
    CelestialBody earth = createCelestialBody(
        "textures/earth.jpg", 
        0.3f,      // Earth size
        12.0f,     // Safe distance from venus (was 6.0f)
        1.0f,      // Earth orbital speed reference
        20.0f      // Earth rotation speed
    );

    // MOON - Orbits Earth
    CelestialBody moon = createCelestialBody(
        "textures/moon.jpg", 
        0.08f,     // Small moon size
        1.2f,      // Distance from Earth (increased from 1.0f)
        4.0f,      // Fast orbit around Earth
        5.0f       // Moon rotation
    );

    // MARS - Fourth planet
    CelestialBody mars = createCelestialBody(
        "textures/mars.jpg", 
        0.16f,     // Mars size
        15.0f,     // Safe distance from Earth (was 7.5f)
        0.8f,      // Slower orbital speed than Earth
        18.0f      // Rotation speed
    );

    // JUPITER - Fifth planet, largest
    CelestialBody jupiter = createCelestialBody(
        "textures/jupiter.jpg", 
        3.36f,     // Large size
        22.0f,     // Safe distance from Mars (was 10.0f)
        0.5f,      // Slower orbital speed
        30.0f      // Fast rotation speed
    );

    // SATURN - Sixth planet with rings
    CelestialBody saturn = createCelestialBody(
        "textures/saturn.jpg", 
        2.82f,     // Large size
        28.0f,     // Safe distance from Jupiter (was 14.0f)
        0.35f,     // Slow orbital speed
        28.0f      // Fast rotation speed
    );

    // URANUS - Seventh planet
    CelestialBody uranus = createCelestialBody(
        "textures/uranus.jpg",
        1.2f,      // Medium size
        34.0f,     // Safe distance from Saturn (was 18.0f)
        0.25f,     // Very slow orbital speed
        -15.0f     // Retrograde rotation
    );

    // NEPTUNE - Outermost planet
    CelestialBody neptune = createCelestialBody(
        "textures/neptune.jpg",
        1.17f,     // Medium size
        40.0f,     // Safe distance from Uranus (was 22.0f)
        0.2f,      // Slowest orbital speed
        18.0f      // Normal rotation speed
    );

    Comet halleysComet = createComet(
        "textures/comet.jpg",
        vec3(0.0f, 0.0f, -20.0f),
        45.0f,
        0.85f
    );

    Comet comet2 = createComet(
        "textures/comet.jpg",
        vec3(0.0f, 0.0f, -20.0f),
        25.0f,
        0.7f
    );
    comet2.orbitAngle = 180.0f; // Start on opposite side

    // Create Saturn's rings
    PlanetRing saturnRings = createSaturnRings();

    // Setup planet selector
    PlanetSelector planetSelector;
    planetSelector.addCelestialBody(&sun, "Sun");
    planetSelector.addCelestialBody(&mercury, "Mercury");
    planetSelector.addCelestialBody(&venus, "Venus");
    planetSelector.addCelestialBody(&earth, "Earth");
    planetSelector.addCelestialBody(&moon, "Moon");
    planetSelector.addCelestialBody(&mars, "Mars");
    planetSelector.addCelestialBody(&jupiter, "Jupiter");
    planetSelector.addCelestialBody(&saturn, "Saturn");
    planetSelector.addCelestialBody(&uranus, "Uranus");
    planetSelector.addCelestialBody(&neptune, "Neptune");

    // Add planet selection mode flag
    bool planetSelectionMode = false;

    // Create list of all celestial bodies for black hole effect
    vector<CelestialBody*> allBodies = {&sun, &mercury, &venus, &earth, &moon, &mars, &jupiter, &saturn, &uranus, &neptune};
    
    // Store reset positions AFTER bodies are created but BEFORE any updates
    BlackHole blackHole;
    blackHole.position = vec3(0.0f, 0.0f, -20.0f); // Black hole center
    blackHole.strength = 0.0f;
    blackHole.active = false;
    blackHole.activationTime = 0.0f;
    
    // Now set up initial orbital positions for normal animation
    updateCelestialBody(mercury, sun.position, 0.0f, 0.0f);
    updateCelestialBody(venus, sun.position, 45.0f, 0.0f);
    updateCelestialBody(earth, sun.position, 90.0f, 0.0f);
    updateCelestialBody(mars, sun.position, 135.0f, 0.0f);
    updateCelestialBody(jupiter, sun.position, 180.0f, 0.0f);
    updateCelestialBody(saturn, sun.position, 225.0f, 0.0f);
    updateCelestialBody(uranus, sun.position, 270.0f, 0.0f);
    updateCelestialBody(neptune, sun.position, 315.0f, 0.0f);
    updateCelestialBody(moon, earth.position, 0.0f, 0.0f);
    
    // Store these as the RESET positions (what we return to with R key)
    blackHole.resetPositions.clear();
    blackHole.resetScales.clear();
    blackHole.resetPositions.push_back(sun.position);          
    blackHole.resetScales.push_back(sun.scale);
    blackHole.resetPositions.push_back(mercury.position);      
    blackHole.resetScales.push_back(mercury.scale);
    blackHole.resetPositions.push_back(venus.position);        
    blackHole.resetScales.push_back(venus.scale);
    blackHole.resetPositions.push_back(earth.position);        
    blackHole.resetScales.push_back(earth.scale);
    blackHole.resetPositions.push_back(moon.position);         
    blackHole.resetScales.push_back(moon.scale);
    blackHole.resetPositions.push_back(mars.position);         
    blackHole.resetScales.push_back(mars.scale);
    blackHole.resetPositions.push_back(jupiter.position);      
    blackHole.resetScales.push_back(jupiter.scale);
    blackHole.resetPositions.push_back(saturn.position);       
    blackHole.resetScales.push_back(saturn.scale);
    blackHole.resetPositions.push_back(uranus.position);       
    blackHole.resetScales.push_back(uranus.scale);
    blackHole.resetPositions.push_back(neptune.position);      
    blackHole.resetScales.push_back(neptune.scale);

    // Setup skybox
    std::vector<std::string> skyboxFaces = {
        "textures/skybox/1.png",
        "textures/skybox/2.png",
        "textures/skybox/3.png",
        "textures/skybox/4.png",
        "textures/skybox/5.png",
        "textures/skybox/6.png"
    };
    Skybox skybox = createSkybox(skyboxFaces);

    // Initialize animation variables
    float spinningCubeAngle = 0.0f;
    float orbAngle = 0.0f;
    float orbRadius = 2.0f;
    float orbHeight = 1.0f;
    float orbSize = 0.2f;

    // Set up texture uniform for the base shader
    glUseProgram(shaders.base);
    glUniform1i(glGetUniformLocation(shaders.base, "texture1"), 0);

    // Initialize timing and input state
    float lastFrameTime = glfwGetTime();
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);
    bool isPaused = false;
    bool wasSpacePressed = false;

    // Input state tracking for X & R keys
    static bool wasXPressed = false;
    static bool wasRPressed = false;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Update timing
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;

        // Handle pause input
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (!wasSpacePressed)
            {
                isPaused = !isPaused;
                wasSpacePressed = true;
            }
        }
        else
        {
            wasSpacePressed = false;
        }
        
        // Handle planet selection with key 3
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            if (!planetSelector.was3Pressed) {
                if (!planetSelectionMode) {
                    // Enter planet selection mode
                    planetSelectionMode = true;
                    camera.firstPerson = false; // Switch to third person for better planet viewing
                    cout << "Entered planet selection mode. Selected: " << planetSelector.getSelectedName() << endl;
                } else {
                    // Cycle to next planet
                    planetSelector.nextSelection();
                }
                planetSelector.was3Pressed = true;
            }
        } else {
            planetSelector.was3Pressed = false;
        }

        // Exit planet selection mode with key 4
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            if (planetSelectionMode) {
                planetSelectionMode = false;
                cout << "Exited planet selection mode" << endl;
            }
        }
        
        // X and R key handling for black hole effect
        // X key to activate black hole
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            if (!wasXPressed && !blackHole.active) {
                blackHole.active = true;
                blackHole.activationTime = glfwGetTime();
                std::cout << "Black hole activated!" << std::endl;
                
                // CRITICAL FIX: Capture CURRENT positions when X is pressed, not stored positions
                blackHole.originalPositions.clear();
                blackHole.originalScales.clear();
                blackHole.originalPositions.push_back(sun.position);          // Current sun position
                blackHole.originalScales.push_back(sun.scale);
                blackHole.originalPositions.push_back(mercury.position);      // Current mercury position
                blackHole.originalScales.push_back(mercury.scale);
                blackHole.originalPositions.push_back(venus.position);        // Current venus position
                blackHole.originalScales.push_back(venus.scale);
                blackHole.originalPositions.push_back(earth.position);        // Current earth position
                blackHole.originalScales.push_back(earth.scale);
                blackHole.originalPositions.push_back(moon.position);         // Current moon position
                blackHole.originalScales.push_back(moon.scale);
                blackHole.originalPositions.push_back(mars.position);         // Current mars position
                blackHole.originalScales.push_back(mars.scale);
                blackHole.originalPositions.push_back(jupiter.position);      // Current jupiter position
                blackHole.originalScales.push_back(jupiter.scale);
                blackHole.originalPositions.push_back(saturn.position);       // Current saturn position
                blackHole.originalScales.push_back(saturn.scale);
                blackHole.originalPositions.push_back(uranus.position);       // Current uranus position
                blackHole.originalScales.push_back(uranus.scale);
                blackHole.originalPositions.push_back(neptune.position);      // Current neptune position
                blackHole.originalScales.push_back(neptune.scale);
                
                wasXPressed = true;
            }
        } else {
            wasXPressed = false;
        }

        // R key to reset
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            if (!wasRPressed) {
                blackHole.active = false;
                blackHole.strength = 0.0f;
                std::cout << "Black hole reset!" << std::endl;
                
                // Reset all bodies to normal orbital positions (not the X-pressed positions)
                sun.position = blackHole.resetPositions[0];
                sun.scale = blackHole.resetScales[0];
                mercury.position = blackHole.resetPositions[1]; 
                mercury.scale = blackHole.resetScales[1];
                venus.position = blackHole.resetPositions[2];
                venus.scale = blackHole.resetScales[2];
                earth.position = blackHole.resetPositions[3];
                earth.scale = blackHole.resetScales[3];
                moon.position = blackHole.resetPositions[4];
                moon.scale = blackHole.resetScales[4];
                mars.position = blackHole.resetPositions[5];
                mars.scale = blackHole.resetScales[5];
                jupiter.position = blackHole.resetPositions[6];
                jupiter.scale = blackHole.resetScales[6];
                saturn.position = blackHole.resetPositions[7];
                saturn.scale = blackHole.resetScales[7];
                uranus.position = blackHole.resetPositions[8];
                uranus.scale = blackHole.resetScales[8];
                neptune.position = blackHole.resetPositions[9];
                neptune.scale = blackHole.resetScales[9];
                
                wasRPressed = true;
            }
        } else {
            wasRPressed = false;
        }

        float animationDt = isPaused ? 0.0f : dt;

        // Update camera for planet selection mode
        if (planetSelectionMode) {
            CelestialBody* selectedBody = planetSelector.getSelectedBody();
            updateCameraForSelectedPlanet(camera, selectedBody, dt);
        }

        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update view matrix
        mat4 viewMatrix = updateViewMatrix(camera);
        GLuint viewMatrixLocation = glGetUniformLocation(shaders.base, "viewMatrix");
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);

        // Render skybox
        renderSkybox(skybox, shaders.skybox, viewMatrix, projectionMatrix);

        // Setup base shader for scene rendering
        glUseProgram(shaders.base);
        
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, &viewMatrix[0][0]);
        
        glUniformMatrix4fv(
            projectionMatrixLocation, 
            1, 
            GL_FALSE, 
            &projectionMatrix[0][0]
        );
        
        glBindVertexArray(vao);

        // Update and render spinning duck (third-person view only)
        spinningCubeAngle += 180.0f * dt;
        if (!camera.firstPerson && !planetSelectionMode)
        {
            GLuint worldMatrixLocation = glGetUniformLocation(
                shaders.base, 
                "worldMatrix"
            );

            mat4 spinningCubeWorldMatrix = translate(
                mat4(1.0f), 
                camera.position + vec3(0.0f, -0.2f, 0.0f)
            ) 
            * rotate(
                mat4(1.0f), 
                radians(spinningCubeAngle), 
                vec3(0.0f, 1.0f, 0.0f)
            )
            * rotate(
                mat4(1.0f),
                radians(1.0f),
                vec3(0.0f, 0.0f, 1.0f)
            )
            * scale(
                mat4(1.0f), 
                vec3(0.0006f, 0.0006f, 0.0006f)
            );

            glUniformMatrix4fv(
                worldMatrixLocation, 
                1, 
                GL_FALSE, 
                &spinningCubeWorldMatrix[0][0]
            );
            
            if (!duckModel.meshes.empty()) {
                glDisable(GL_CULL_FACE);
                duckModel.Draw(shaders.base);
                glEnable(GL_CULL_FACE);
            }
        }

        // Update celestial body positions and handle black hole effect
        orbAngle += 20.0f * animationDt;
        vec3 sunPosition = vec3(0.0f, 0.0f, -20.0f);

        // FIXED: Proper black hole effect implementation
        if (blackHole.active) {
            float elapsed = glfwGetTime() - blackHole.activationTime;
            float effectDuration = 6.0f; // 6 second effect for better visibility
            blackHole.strength = std::min(1.0f, elapsed / effectDuration);
            
            // Apply black hole effect to all bodies - shrink to COMPLETELY INVISIBLE
            float shrinkFactor = std::max(0.0f, 1.0f - blackHole.strength); // Goes from 1.0 to 0.0 (completely invisible)
            
            // Linear interpolation function
            auto lerp = [](const vec3& a, const vec3& b, float t) -> vec3 {
                return a + t * (b - a);
            };
            
            // Apply effect to ALL bodies including the sun - they all go to the black hole center
            sun.position = lerp(blackHole.originalPositions[0], blackHole.position, blackHole.strength);
            sun.scale = blackHole.originalScales[0] * shrinkFactor;
            
            mercury.position = lerp(blackHole.originalPositions[1], blackHole.position, blackHole.strength);
            mercury.scale = blackHole.originalScales[1] * shrinkFactor;
            
            venus.position = lerp(blackHole.originalPositions[2], blackHole.position, blackHole.strength);
            venus.scale = blackHole.originalScales[2] * shrinkFactor;
            
            earth.position = lerp(blackHole.originalPositions[3], blackHole.position, blackHole.strength);
            earth.scale = blackHole.originalScales[3] * shrinkFactor;
            
            moon.position = lerp(blackHole.originalPositions[4], blackHole.position, blackHole.strength);
            moon.scale = blackHole.originalScales[4] * shrinkFactor;
            
            mars.position = lerp(blackHole.originalPositions[5], blackHole.position, blackHole.strength);
            mars.scale = blackHole.originalScales[5] * shrinkFactor;
            
            jupiter.position = lerp(blackHole.originalPositions[6], blackHole.position, blackHole.strength);
            jupiter.scale = blackHole.originalScales[6] * shrinkFactor;
            
            saturn.position = lerp(blackHole.originalPositions[7], blackHole.position, blackHole.strength);
            saturn.scale = blackHole.originalScales[7] * shrinkFactor;
            
            uranus.position = lerp(blackHole.originalPositions[8], blackHole.position, blackHole.strength);
            uranus.scale = blackHole.originalScales[8] * shrinkFactor;
            
            neptune.position = lerp(blackHole.originalPositions[9], blackHole.position, blackHole.strength);
            neptune.scale = blackHole.originalScales[9] * shrinkFactor;
            
            // Don't do normal orbital updates during black hole effect - COMPLETELY override positions
        } else {
            // Normal celestial body updates only when black hole is not active
            updateCelestialBody(sun, sun.position, orbAngle, animationDt);
            updateCelestialBody(mercury, sun.position, orbAngle, animationDt);
            updateCelestialBody(venus, sun.position, orbAngle, animationDt);
            updateCelestialBody(earth, sun.position, orbAngle, animationDt);
            updateCelestialBody(mars, sun.position, orbAngle, animationDt);
            updateCelestialBody(jupiter, sun.position, orbAngle, animationDt);
            updateCelestialBody(saturn, sun.position, orbAngle, animationDt);
            updateCelestialBody(uranus, sun.position, orbAngle, animationDt);
            updateCelestialBody(neptune, sun.position, orbAngle, animationDt);
            updateCelestialBody(moon, earth.position, orbAngle, animationDt);
        }

        // Update comets
        updateComet(halleysComet, animationDt, sun.position);
        updateComet(comet2, animationDt, sun.position);

        // Collect all planet positions and radii for shadow calculations
        vector<vec3> planetPositions = {
            mercury.position, venus.position, earth.position, mars.position,
            jupiter.position, saturn.position, uranus.position, neptune.position,
            moon.position  // Add moon for shadow calculations
        };
        vector<float> planetRadii = {
            mercury.scale.x, venus.scale.x, earth.scale.x, mars.scale.x,
            jupiter.scale.x, saturn.scale.x, uranus.scale.x, neptune.scale.x,
            moon.scale.x  // Add moon radius for shadow calculations
        };

        // Render all celestial bodies in order from sun outward - BUT ONLY IF VISIBLE
        // Check if each body is large enough to be visible (scale > 0.01f means visible)
        if (sun.scale.x > 0.01f) {
            renderCelestialBody(sun, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, true);
        }
        if (mercury.scale.x > 0.01f) {
            renderCelestialBody(mercury, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }
        if (venus.scale.x > 0.01f) {
            renderCelestialBody(venus, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }
        if (earth.scale.x > 0.01f) {
            renderCelestialBody(earth, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }
        if (mars.scale.x > 0.01f) {
            renderCelestialBody(mars, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }
        if (jupiter.scale.x > 0.01f) {
            renderCelestialBody(jupiter, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }
        if (saturn.scale.x > 0.01f) {
            renderCelestialBody(saturn, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }
        // Render Saturn's rings immediately after Saturn only if Saturn is visible
        //if (saturn.scale.x > 0.01f) {
        //    renderPlanetRings(saturnRings, saturn, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position);
        //}
        if (uranus.scale.x > 0.01f) {
            renderCelestialBody(uranus, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }
        if (neptune.scale.x > 0.01f) {
            renderCelestialBody(neptune, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }
        if (moon.scale.x > 0.01f) {
            renderCelestialBody(moon, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        }

        // Render comet trails first (so they appear behind comet heads)
        renderCometTrail(halleysComet, shaders.base, viewMatrix, projectionMatrix);
        renderCometTrail(comet2, shaders.base, viewMatrix, projectionMatrix);

        // Render comet heads
        renderCelestialBody(halleysComet.body, shaders.orb, viewMatrix, projectionMatrix, 
                        sun.position, camera.position, false, planetPositions, planetRadii);
        renderCelestialBody(comet2.body, shaders.orb, viewMatrix, projectionMatrix, 
                        sun.position, camera.position, false, planetPositions, planetRadii);

        // Render selection indicator if in planet selection mode
        if (planetSelectionMode) {
            CelestialBody* selectedBody = planetSelector.getSelectedBody();
            renderSelectionIndicator(selectedBody, shaders.base, viewMatrix, projectionMatrix);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Handle keyboard input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
        
        // Only allow manual camera mode switching when not in planet selection mode
        if (!planetSelectionMode) {
            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            {
                camera.firstPerson = true;
            }
            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            {
                camera.firstPerson = false;
            }
        }

        // Update camera from mouse input (only when not in planet selection mode)
        if (!planetSelectionMode) {
            double mousePosX, mousePosY;
            glfwGetCursorPos(window, &mousePosX, &mousePosY);
            double dx = mousePosX - lastMousePosX;
            double dy = mousePosY - lastMousePosY;
            lastMousePosX = mousePosX;
            lastMousePosY = mousePosY;

            updateCameraAngles(
                camera, 
                static_cast<float>(dx), 
                static_cast<float>(dy), 
                dt
            );
            updateCameraPosition(camera, window, dt);

            // Handle arrow key camera control
            const float arrowLookSpeed = 60.0f;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            {
                camera.horizontalAngle += arrowLookSpeed * dt;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            {
                camera.horizontalAngle -= arrowLookSpeed * dt;
            }
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            {
                camera.verticalAngle += arrowLookSpeed * dt;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            {
                camera.verticalAngle -= arrowLookSpeed * dt;
            }
            
            // Update camera look direction based on angles
            float theta = radians(camera.horizontalAngle);
            float phi = radians(camera.verticalAngle);
            camera.lookAt = vec3(cos(phi) * cos(theta), sin(phi), -cos(phi) * sin(theta));
        }
    }

    // Cleanup
    glfwTerminate();
    return 0;
}