#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix = mat4(1.0);
uniform mat4 projectionMatrix = mat4(1.0);

out vec3 Normal;
out vec2 TexCoords;

void main()
{
    Normal = mat3(transpose(inverse(worldMatrix))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(aPos, 1.0);
}
