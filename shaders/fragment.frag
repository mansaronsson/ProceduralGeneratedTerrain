#version 330 core
out vec4 outColor;

in vec3 o_normal;
in vec3 pos;
in vec3 lod_color;
in float vert_biome;

uniform bool colorDistance;
uniform bool biomeColor;

vec3 lightdir = vec3(1.0f, 1.0f, 0.0f);

vec3 getDesertColor() {
	vec3 color = vec3(1.0f, 0.8f, 0.0f);

	return color;
}

vec3 getPlainsColor() {
	vec3 color = vec3(0.2f, 0.2f, 0.7f); // blue

	float ss = smoothstep(0.0f, 0.5f, pos.y);
	color = ss * vec3(0.1f, 0.5f, 0.1f) + (1 - ss) * color; // green

	ss = smoothstep(8.0f, 13.5f, pos.y);
	color = ss * vec3(0.54f, 0.38f, 0.08f) + (1 - ss) * color; // brown


	return color;
}

vec3 getCrystalMountainColor() {
	vec3 color = vec3(0.2f, 0.2f, 0.7f); // blue

	float ss = smoothstep(0.0f, 0.5f, pos.y);
	color = ss * vec3(0.2f, 0.7f, 0.2f) + (1 - ss) * color; // green

	ss = smoothstep(5.0f, 9.5f, pos.y);
	color = ss * vec3(0.6f, 0.4f, 0.3f) + (1 - ss) * color; // brown 
	
	ss = smoothstep(12.0f, 18.0f, pos.y);
	color = ss * vec3(0.43f, 0.48f, 0.5f) + (1 - ss) * color; // gray

	ss = smoothstep(25.0f, 27.0f, pos.y);
	color = ss * vec3(1.0f, 1.0f, 1.0f) + (1 - ss) * color; // white

	return color;
}

void main() {
	vec3 normal = normalize(o_normal);
	lightdir = normalize(lightdir);
	float k = max(dot(normal, lightdir), 0.0);
	
	// Desert color
	vec3 color = getDesertColor();

	// Plains color
	float ss = smoothstep(0.5f, 1.0f, vert_biome);
	color = ss * getPlainsColor() + (1-ss) * color;

	// Crystal mountain color
	ss = smoothstep(1.5f, 2.0f, vert_biome);
	color = ss * getCrystalMountainColor() + (1-ss) * color;

	// LOD color
	color = int(colorDistance) * lod_color + (1 - int(colorDistance)) * color;

	// Biome color
	ss = smoothstep(0.0f, 2.0f, vert_biome);	// second argument should be number of biomes - 1
	color = int(biomeColor) * (ss * vec3(1.0f, 0.0f, 0.0f) +  vec3(0.0f, 0.0f, 1.0f)) + (1 - int(biomeColor)) * color;

	outColor = k * vec4(color, 1.0);
}
