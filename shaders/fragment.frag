#version 330 core
out vec4 outColor;

in vec3 o_normal;
in vec3 pos;
vec3 lightdir = vec3(1.0, 1.0, 0.0);

void main() {
	vec3 normal = normalize(o_normal);
	lightdir = normalize(lightdir);
	float k = max(dot(normal, lightdir), 0.0);

	vec3 color = vec3(0.2, 0.9, 0.3); //vec3(1.0, 0.2, 0.8);
	if(pos.y > 1.35) {
		color = vec3(1.0,1.0,1.0);
	}
	else if(pos.y > 0.6) {
		color = vec3(0.6, 0.4, 0.5);
	}
	else if(pos.y > 0.0) {
		color = vec3(0.3, 0.8, 0.2);
	}
	else if(pos.y < -1.5) {
		color = vec3(0.2, 0.2, 0.7);
	}
	

	outColor = vec4(0.1, 0.1, 0.1, 1.0) +  k *  vec4(color, 1.0);
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