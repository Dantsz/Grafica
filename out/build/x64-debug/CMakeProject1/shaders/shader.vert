#version 410 core

layout(location = 0) in vec3 vertex_position;

void main() {
    gl_Position = vec4(vertex_position * 2, 1.0);
}
