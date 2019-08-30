#version 330 core

out vec2 coords;

layout(location = 0) in vec4 vertex;

uniform mat4 projection;

void main() 
{
    coords = vertex.xy;
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
}