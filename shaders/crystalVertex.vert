#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;


uniform mat4 M,V,P;
void main() {
	gl_Position = P * V * M * vec4(position, 1.0);
}