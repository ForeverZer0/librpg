#version 330 core

in vec2 coords;
out vec4 color;

uniform sampler2D image;

void main() {    
    color = texture(image, coords);
}