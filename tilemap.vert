#version 330 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in mat4 model;

uniform mat4 projection;
uniform vec2 origin;

out vec2 coords;

void main() {
    coords      = vertex.zw;
    mat4 test = model;
    test[3][0] -= origin.x;
    test[3][1] -= origin.y;
    gl_Position = projection * test * vec4(vertex.xy, 0.0, 1.0);
}