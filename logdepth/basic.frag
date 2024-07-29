#version 330 core

uniform vec3 color;

uniform float farPlane;

in float vFragDepth;

out vec4 outColor;


void main() {
    outColor = vec4(color, 1.0);

    // TODO: optimize like in three.js / outerra: log2, put the whole 1/log(1+farPlane) to a uniform
    gl_FragDepth = log( vFragDepth ) / log( 1.0 + farPlane );
}
