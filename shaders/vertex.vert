#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 MV, P;
void main() {
	gl_Position = P * MV * vec4(position, 1.0);
}