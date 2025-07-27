#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec2 TexCoord;
out vec3 FragPos;  // Fragment position in world space
out vec3 Normal;   // Normal in world space

void main() {
    vec4 worldPos = worldMatrix * vec4(aPos, 1.0);
    FragPos = vec3(worldPos);
    
    // For a sphere, the normal is the same as the position (normalized)
    Normal = normalize(mat3(worldMatrix) * aPos);
    
    TexCoord = aTexCoord;
    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
