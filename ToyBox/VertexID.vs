#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int offset;

flat out int ID;

void main() {
	ID = gl_VertexID + offset + 1;
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}