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
		&fixedViewPos[0]
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

    // Replace the celestial body creation section in your main() function with this:

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


// Create Saturn's rings
PlanetRing saturnRings = createSaturnRings();

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
        float animationDt = isPaused ? 0.0f : dt;

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
        if (!camera.firstPerson)
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

        // Update celestial bodies - Complete solar system animation
        orbAngle += 20.0f * animationDt;
        vec3 sunPosition = vec3(0.0f, 0.0f, -20.0f);
        
        // Update all celestial bodies with realistic orbital mechanics
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

        // Collect all planet positions and radii for shadow calculations
        vector<vec3> planetPositions = {
            mercury.position, venus.position, earth.position, mars.position,
            jupiter.position, saturn.position, uranus.position, neptune.position
        };
        vector<float> planetRadii = {
            mercury.scale.x, venus.scale.x, earth.scale.x, mars.scale.x,
            jupiter.scale.x, saturn.scale.x, uranus.scale.x, neptune.scale.x
        };

        // Render all celestial bodies in order from sun outward
        renderCelestialBody(sun, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, true);
        renderCelestialBody(mercury, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        renderCelestialBody(venus, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        renderCelestialBody(earth, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        renderCelestialBody(mars, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        renderCelestialBody(jupiter, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        renderCelestialBody(saturn, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        // Render Saturn's rings immediately after Saturn
        renderPlanetRings(saturnRings, saturn, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position);
        renderCelestialBody(uranus, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        renderCelestialBody(neptune, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);
        renderCelestialBody(moon, shaders.orb, viewMatrix, projectionMatrix, sun.position, camera.position, false, planetPositions, planetRadii);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Handle keyboard input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            camera.firstPerson = true;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            camera.firstPerson = false;
        }

        // Update camera from mouse input
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

    // Cleanup
    glfwTerminate();
    return 0;
}
