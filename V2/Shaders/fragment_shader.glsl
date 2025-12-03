#version 410 core

in vec3 vColor;
in vec2 vUV;

out vec4 FragColor;

uniform sampler2D uTexture;

void main() {
    vec4 texColor = texture(uTexture, vUV);
    FragColor = texColor * vec4(vColor, 1.0);
}
