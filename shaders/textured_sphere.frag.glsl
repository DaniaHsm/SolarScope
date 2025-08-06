#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D texture1;
uniform vec3 lightPos;      // Sun's position
uniform vec3 viewPos;       // Camera position - ADD THIS NEW UNIFORM
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

// NEW FUNCTION: Calculate atmospheric glow color based on planet type
vec3 getAtmosphereColor(vec2 texCoord) {
    // Sample the texture to determine planet type based on dominant colors
    vec4 texColor = texture(texture1, texCoord);
    
    // Earth - blue atmosphere
    if (texColor.b > 0.3 && texColor.g > 0.3) {
        return vec3(0.3, 0.6, 1.0); // Light blue
    }
    // Mars - thin reddish atmosphere
    else if (texColor.r > texColor.g && texColor.r > texColor.b) {
        return vec3(1.0, 0.4, 0.2); // Orange-red
    }
    // Venus - thick yellowish atmosphere
    else if (texColor.r > 0.6 && texColor.g > 0.6 && texColor.b < 0.3) {
        return vec3(1.0, 0.8, 0.3); // Yellow-orange
    }
    // Gas giants - use dominant color with slight blue tint
    else {
        return mix(texColor.rgb, vec3(0.5, 0.7, 1.0), 0.3);
    }
}

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    
    if (isSun) {
        // Sun is self-illuminating with slight glow
        vec3 normal = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        float rim = 1.0 - max(dot(normal, viewDir), 0.0);
        rim = pow(rim, 2.0);
        
        vec3 glowColor = vec3(1.0, 0.8, 0.4); // Warm sun glow
        FragColor = texColor + vec4(glowColor * rim * 0.3, 0.0);
    } else {
        // Calculate lighting for planets
        vec3 normal = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 viewDir = normalize(viewPos - FragPos);
        
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
        
        // NEW: Calculate atmospheric rim lighting
        float rim = 1.0 - max(dot(normal, viewDir), 0.0);
        rim = pow(rim, 3.0); // Make rim more focused
        
        // Get atmosphere color for this planet
        vec3 atmosphereColor = getAtmosphereColor(TexCoord);
        
        // Apply atmospheric glow (stronger on lit side)
        float glowStrength = 0.4 * (0.5 + 0.5 * diff);
        vec3 atmosphericGlow = atmosphereColor * rim * glowStrength;
        
        // Combine lighting with texture
        vec3 result = (ambient + diff) * texColor.rgb + atmosphericGlow;
        FragColor = vec4(result, texColor.a);
    }
}