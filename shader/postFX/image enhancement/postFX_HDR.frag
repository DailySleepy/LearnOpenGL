#version 460 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform bool useHDR;
uniform float exposure;

void main(){
	vec3 color = vec3(texture(screenTexture, texCoord));
	vec3 mapped = color;
	
	if(useHDR) {
		//Reinhard
		mapped = color / (color + vec3(1));

		//exposure
		mapped = vec3(1) - exp(-color * exposure);

		//ACES
		vec3 a = vec3(2.51);
		vec3 b = vec3(0.03);
		vec3 c = vec3(2.43);
		vec3 d = vec3(0.59);
		vec3 e = vec3(0.14);
		mapped = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);

		//gamma
		//mapped = pow(mapped, 1.0 / vec3(2.2));
	}

	FragColor = vec4(mapped, 1);
}
