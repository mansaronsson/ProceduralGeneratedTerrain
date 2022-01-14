#version 330 core
out vec4 outColor;

in vec3 o_normal;
in vec3 pos;
in vec2 st;

vec3 lightdir = vec3(1.0, 1.0, 0.0);

uniform sampler2D barkTexture;

void main() {
	vec3 normal = normalize(o_normal);
	lightdir = normalize(lightdir);
	float k = max(dot(normal, lightdir), 0.0);
//	k = 1.0f;
	k = normal.z + 0.5;
	vec3 color = vec3(0.2, 0.9, 0.3); //vec3(1.0, 0.2, 0.8);

	outColor = vec4(0.1, 0.1, 0.1, 1.0) + k * vec4(color, 1.0);

	outColor = vec4(0.1, 0.1, 0.1, 1.0) + k * texture(barkTexture, st);
}

/*#version 330 core
out vec4 outColor;

in vec3 o_normal;
in vec3 pos;
vec3 lightpos = vec3(1.0, 1.0, 1.0);

void main() {
	vec3 normal = normalize(o_normal);
	vec3 lightdir = normalize(lightpos - pos);
	float k = max(dot(normal, lightdir), 0.0);

	outColor = vec4(0.1, 0.1, 0.1, 1.0) +  k *  vec4(1.0, 0.2, 0.8, 1.0);
}*/