#version 330 core
out vec4 outColor;

in vec2 st;

vec3 lightdir = vec3(1.0, 1.0, 0.0);

uniform sampler2D barkTexture;

void main() {
	outColor =  texture(barkTexture, st);
	if(outColor.a < 0.1)
		discard;
}
