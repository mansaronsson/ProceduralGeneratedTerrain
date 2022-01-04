#version 330 core
out vec4 outColor;

in vec3 o_normal;
in vec3 pos;
in vec3 o_color;

vec3 lightdir = vec3(1.0f, 1.0f, 0.0f);

void main() {
	vec3 normal = normalize(o_normal);
	lightdir = normalize(lightdir);
	float k = max(dot(normal, lightdir), 0.0);

	outColor = (0.5f + k * 0.5f) * vec4(0.8f, 0.0f, 0.4f, 0.8f);
}