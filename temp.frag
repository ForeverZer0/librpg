#version 330 core

in vec2 coords;
out vec4 result;

uniform sampler2D image;

void main() {
    result = vec4(1.0, 0.5, 0.25, 1.0);
}