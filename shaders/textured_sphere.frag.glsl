#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightPos;      // Sun's position
uniform bool isSun;         // Whether this object is the sun

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    
    if (isSun) {
        // Sun is self-illuminating
        FragColor = texColor;
    } else {
        // Calculate lighting for planets
        vec3 normal = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        
        // Calculate diffuse lighting (day/night effect)
        float diff = max(dot(normal, lightDir), 0.0);
        
        // Ambient light (for slightly visible night side)
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * vec3(1.0);
        
        // Combine lighting with texture
        vec3 result = (ambient + diff) * texColor.rgb;
        FragColor = vec4(result, texColor.a);
    }
}
