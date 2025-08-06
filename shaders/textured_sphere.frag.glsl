#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightPos;      // Sun's position
uniform bool isSun;         // Whether this object is the sun

// Shadow casting uniforms
#define MAX_PLANETS 9
uniform vec3 planetPositions[MAX_PLANETS];  // Positions of all planets
uniform float planetRadii[MAX_PLANETS];     // Radii of all planets
uniform int numPlanets;                     // Number of planets

bool isInShadow() {
    if (isSun) return false;
    
    vec3 lightDir = normalize(lightPos - FragPos);
    float distanceToLight = length(lightPos - FragPos);
    
    // Check each planet for potential shadowing
    for (int i = 0; i < numPlanets; i++) {
        vec3 planetToFragment = FragPos - planetPositions[i];
        float planetRadius = planetRadii[i];
        
        // Skip if this is our own planet
        if (length(planetToFragment) < planetRadius * 1.1) continue;
        
        // Calculate closest point on ray to planet center
        float t = dot(lightDir, planetPositions[i] - FragPos);
        vec3 closestPoint = FragPos + lightDir * t;
        
        // Check if closest point is between fragment and light
        if (t > 0 && t < distanceToLight) {
            float dist = length(closestPoint - planetPositions[i]);
            if (dist < planetRadius) {
                return true;
            }
        }
    }
    return false;
}

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    
    if (isSun) {
        // Sun is self-illuminating
        FragColor = texColor;
    } else {
        // Calculate lighting for planets
        vec3 normal = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        
        // Check for shadows
        bool shadowed = isInShadow();
        
        // Calculate diffuse lighting (day/night effect)
        float diff = max(dot(normal, lightDir), 0.0);
        if (shadowed) {
            diff *= 0.1; // Reduce lighting significantly in shadowed areas
        }
        
        // Ambient light (for slightly visible night side)
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * vec3(1.0);
        
        // Combine lighting with texture
        vec3 result = (ambient + diff) * texColor.rgb;
        FragColor = vec4(result, texColor.a);
    }
}
