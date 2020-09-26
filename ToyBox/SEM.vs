#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 vN;
out vec3 eye;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	mat4 modelViewMat = view * model;
	vec4 p = vec4(position, 1.0);

	eye = vec3(modelViewMat * p);
	vN = mat3(transpose(inverse(modelViewMat))) * normal;

	gl_Position = projection * modelViewMat * p;
}