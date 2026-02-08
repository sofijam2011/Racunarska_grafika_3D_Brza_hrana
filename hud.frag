#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec4 uColor;
uniform sampler2D uTexture;
uniform int uUseTexture;

void main() {
    if (uUseTexture == 1) {
        vec4 texColor = texture(uTexture, TexCoord);
        FragColor = texColor * uColor;
    } else {
        FragColor = uColor;
    }
}