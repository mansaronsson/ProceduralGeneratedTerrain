#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 in_st;
layout (location = 3) in mat4 modelMatrix;

out vec2 st;

uniform mat4 M,V,P;
void main() {
	gl_Position = P * V * modelMatrix * vec4(position, 1.0);
	st = in_st;
}