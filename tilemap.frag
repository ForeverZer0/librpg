#version 330 core

in vec2 coords;
out vec4 result;

uniform sampler2D image;

void main() {
    result =  texture(image, coords);
}