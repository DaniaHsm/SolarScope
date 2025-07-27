#version 330 core
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightPos;  // Light position in world space
uniform vec3 viewPos;   // Camera position

void main()
{
    // Ambient lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(1.0);
    
    // Diffuse lighting
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(0.7); // Reduced diffuse intensity
    
    // Combine lighting
    vec3 lighting = ambient + diffuse;
    
    // Get texture color and apply lighting
    vec4 texColor = texture(texture1, TexCoords);
    vec3 result = lighting * texColor.rgb;
    
    // Ensure we're not exceeding maximum brightness
    result = min(result, vec3(1.0));
    
    FragColor = vec4(result, texColor.a);
}
