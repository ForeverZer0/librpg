#version 330 core

layout(location = 0) in vec4 vertex;
layout(location = 1) in mat4 model;

uniform mat4 projection;
out vec2 coords;

void main() {
    coords      = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}