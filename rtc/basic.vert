#version 330 core

uniform mat4 mvp;

uniform mat4 my_mvp;

in vec3 vertexPosition;

void main() {
    //gl_Position = mvp * vec4(vertexPosition, 1.0);
    gl_Position = my_mvp * vec4(vertexPosition, 1.0);
}
