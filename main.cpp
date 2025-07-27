#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <glm/common.hpp>
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

using namespace glm;
using namespace std;

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

struct ShaderPrograms
{
    int base;
    unsigned int skybox;
    unsigned int orb;
};

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

std::string getSkyboxVertexShaderSource()
{
    return readFile("shaders/skybox_vertex.glsl");
}

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

std::string getTexturedSphereVertexShaderSource()
{
    return readFile("shaders/textured_sphere.vert.glsl");
}

std::string getTexturedSphereFragmentShaderSource()
{
    return readFile("shaders/textured_sphere.frag.glsl");
}

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

struct Camera
{
    vec3 position;
    vec3 lookAt;
    vec3 up;
    float speed;
    float fastSpeed;
    float horizontalAngle;
    float verticalAngle;
    bool firstPerson;
};

Camera setupCamera()
{
    Camera camera;
    camera.position = vec3(0.6f, 1.0f, 10.0f);
    camera.lookAt = vec3(0.0f, 0.0f, -1.0f);
    camera.up = vec3(0.0f, 1.0f, 0.0f);
    camera.speed = 3.0f;
    camera.fastSpeed = 6.0f;
    camera.horizontalAngle = 90.0f;
    camera.verticalAngle = 0.0f;
    camera.firstPerson = true;
    return camera;
}

mat4 updateViewMatrix(const Camera &camera)
{
    if (camera.firstPerson)
    {
        return lookAt(camera.position, camera.position + camera.lookAt, camera.up);
    }
    else
    {
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

struct CelestialBody
{
    GLuint vao;
    GLuint texture;
    unsigned int indexCount;
    vec3 position;
    vec3 scale;
    float rotationAngle;
    float rotationSpeed;
    float orbitRadius;
    float orbitSpeed;
};

void generateSphereVerticesAndUVs(
	unsigned int rings,
    unsigned int sectors,
    std::vector<vec3> &vertices,
	std::vector<vec2> &uvs)
{
    float const R = 1.0f / float(rings - 1);
    float const S = 1.0f / float(sectors - 1);

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
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
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

struct Skybox
{
    GLuint vao;
    GLuint vbo;
    GLuint texture;
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
    bool isSun = false)
{
	glDisable(GL_CULL_FACE);
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, body.texture);
    glUniform1i(glGetUniformLocation(shader, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shader, "isSun"), isSun ? 1 : 0);

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
	glEnable(GL_CULL_FACE);
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

    // Setup celestial bodies
    CelestialBody sun = createCelestialBody(
		"textures/sun.jpg", 
		2.0f, 
		0.0f, 
		0.0f, 
		15.0f
	);
    sun.position = vec3(0.0f, 0.0f, -20.0f);

    CelestialBody earth = createCelestialBody(
		"textures/earth.jpg", 
		0.3f, 
		5.0f, 
		1.0f, 
		20.0f
	);
	
    CelestialBody moon = createCelestialBody(
		"textures/moon.jpg", 
		0.08f, 
		1.0f, 
		4.0f, 
		5.0f
	);

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

        // Update and render spinning cube (third-person view only)
        spinningCubeAngle += 180.0f * dt;
        if (!camera.firstPerson)
        {
            GLuint worldMatrixLocation = glGetUniformLocation(
				shaders.base, 
				"worldMatrix"
			);

            mat4 spinningCubeWorldMatrix = translate(
				mat4(1.0f), 
				camera.position
			) 
			* rotate(
				mat4(1.0f), 
				radians(spinningCubeAngle), 
				vec3(0.0f, 1.0f, 0.0f)
			) 
			* scale(
				mat4(1.0f), 
				vec3(0.1f, 0.1f, 0.1f)
			);

            glUniformMatrix4fv(
				worldMatrixLocation, 
				1, 
				GL_FALSE, 
				&spinningCubeWorldMatrix[0][0]
			);
			glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Update celestial bodies
        orbAngle += 20.0f * animationDt;
        vec3 sunPosition = vec3(0.0f, 0.0f, -20.0f);
        updateCelestialBody(sun, sun.position, orbAngle, animationDt);
        updateCelestialBody(earth, sun.position, orbAngle, animationDt);
        updateCelestialBody(moon, earth.position, orbAngle, animationDt);

        // Render celestial bodies
        renderCelestialBody(sun, 
			shaders.orb, 
			viewMatrix, 
			projectionMatrix, 
			sun.position, 
			camera.position,
			true  // This is the sun
		);

        renderCelestialBody(earth, 
			shaders.orb, 
			viewMatrix, 
			projectionMatrix, 
			sun.position, 
			camera.position,
			false  // Not the sun
		);

        renderCelestialBody(moon, 
			shaders.orb, 
			viewMatrix, 
			projectionMatrix, 
			sun.position, 
			camera.position,
			false  // Not the sun
		);

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
    }

    glfwTerminate();

    return 0;
}
