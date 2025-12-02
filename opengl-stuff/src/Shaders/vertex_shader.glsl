#version 410 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
/* 
Vertex shader runs once per vertex
# takes VAO/VBO data
# outputs a position on the screen


# Reads the vertex attribute from slot 0 -> aPos
# converts (x, y) to a vec4
# writes it to gl_Position (gpu uses this to place the vertex on the screen)
*/