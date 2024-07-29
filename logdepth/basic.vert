#version 330 core

uniform mat4 mvp;


in vec3 vertexPosition;

out float vFragDepth;

void main() {
    gl_Position = mvp * vec4(vertexPosition, 1.0);

    vFragDepth = 1.0 + gl_Position.w;
}
