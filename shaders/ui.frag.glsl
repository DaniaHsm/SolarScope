#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D ourTexture;
uniform float alpha;

void main() {
    vec4 texColor = texture(ourTexture, TexCoord);
    if(texColor.a < 0.1) {
        discard;
    }
    FragColor = vec4(texColor.rgb, texColor.a * alpha);
}
