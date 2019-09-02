#version 330 core
precision highp float;

layout(location = 0) in vec2 vertex[6];
layout(location = 6) in mat4 model;

uniform mat4 projection;
uniform vec2 origin;

const vec2 blah[6] = vec2[](
    vec2(0.0, 1.0), 
    vec2(1.0, 0.0), 
    vec2(0.0, 0.0), 
    vec2(0.0, 1.0), 
    vec2(1.0, 1.0), 
    vec2(1.0, 0.0)
);

out vec2 coords;

void main() {
    coords      = vertex[gl_VertexID];
    mat4 translated = model;
    translated[3][0] -= origin.x;
    translated[3][1] -= origin.y;
    gl_Position = projection * translated * vec4(blah[gl_VertexID], 0.0, 1.0);
}