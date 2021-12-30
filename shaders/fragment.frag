#version 330 core
out vec4 outColor;

in vec3 o_normal;
in vec3 pos;
in vec3 lod_color;
flat in int vert_biome;

vec3 lightdir = vec3(1.0, 1.0, 0.0);
//vec3 lightpos = vec3(0, 10, 0);
uniform bool colorDistance;
uniform bool biomeColor;


void main() {
	vec3 normal = normalize(o_normal);
//	vec3 lightdir = lightpos - pos;
	lightdir = normalize(lightdir);
	float k = max(dot(normal, lightdir), 0.0);
	
	// Terrain colors
	vec3 color = vec3(0.2, 0.2, 0.7);; // blue

	float f = smoothstep(-1.5f, -1.4f, pos.y);
	color = f * vec3(0.2, 0.7, 0.2) + (1 - f) * color; // green

	f = smoothstep(0.0f, 0.5f, pos.y);
	color = f * vec3(0.6, 0.4, 0.5) + (1 - f) * color; // gray

	f = smoothstep(1.5f, 2.4f, pos.y);
	color = f * vec3(1.0,1.0,1.0) + (1 - f) * color; // white

	// LOD colors
	color = int(colorDistance) * lod_color + (1 - int(colorDistance)) * color;

	// Biome colors
	f = smoothstep(0, 1, vert_biome);	// second argument should be number of biomes - 1
	color = int(biomeColor) * (f * vec3(0.0f, 0.0f, 1.0f) + (1-f) * vec3(1.0f, 0.0f, 1.0f)) + (1 - int(biomeColor)) * color;

	outColor = k * vec4(color, 1.0);
}
