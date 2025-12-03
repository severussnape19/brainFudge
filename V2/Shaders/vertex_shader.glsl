#version 410 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec2 aUV;

out vec3 vColor;
out vec2 vUV;

uniform mat4 uModel;

void main() {
    gl_Position = uModel * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
    vUV = aUV;
}
