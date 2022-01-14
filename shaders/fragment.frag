#version 330 core
out vec4 outColor;

in vec3 o_normal;
in vec3 pos;
in vec3 o_color;
in vec3 heatcolor, moistcolor, biomecolor, normalcolor;

vec3 lightdir = vec3(1.0, 1.0, 0.0);
//vec3 lightpos = vec3(0, 10, 0);
uniform bool colorDistance, heat, moist, biome, normalC;



void main() {
	vec3 normal = normalize(o_normal);
//	vec3 lightdir = lightpos - pos;
	lightdir = normalize(lightdir);
	float k = max(dot(normal, lightdir), 0.0);
	
	vec3 color = vec3(0.2, 0.9, 0.3); //vec3(1.0, 0.2, 0.8);
	if(pos.y > 2.0) {
		color = vec3(1.0,1.0,1.0); //white
	}
	else if(pos.y > -0.5) {
		color = vec3(0.6, 0.4, 0.5); //gray
	}
	else if(pos.y > -0.9) {
		color = vec3(0.3, 0.8, 0.2); //green
	}
	else if(pos.y < -1.5) {
		color = vec3(0.2, 0.2, 0.7); //blue
	}
	
	if(colorDistance)
		outColor = k * vec4(o_color, 1.0f);
	else if(heat)
		outColor = k * vec4(heatcolor, 1.0f);
	else if(moist)
		outColor = k * vec4(moistcolor, 1.0f);
	else if(biome)
		outColor = k * vec4(biomecolor, 1.0f);
	else if(normalC)
		outColor = k * vec4(normalcolor, 1.0f);
	else 
		outColor = vec4(0.1, 0.1, 0.1, 1.0) + k * vec4(color, 1.0);

	//outColor = k * vec4(1.0, 0.2, 0.8, 1.0);
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


	//if 0 - 1 based noise is used 

	vec3 color = vec3(0.2, 0.9, 0.3); //vec3(1.0, 0.2, 0.8);
	if(pos.y > 0.9) {
		color = vec3(1.0,1.0,1.0); //white
	}
	else if(pos.y > 0.55) {
		color = vec3(0.6, 0.4, 0.5); //gray
	}
	else if(pos.y > 4) {
		color = vec3(0.3, 0.8, 0.2); //green
	}
	else if(pos.y < 0.2) {
		color = vec3(0.2, 0.2, 0.7); //blue
	}
}*/