#version 460 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform bool useBloom;
uniform bool useHDR;
uniform float bloomIntensity;

void main(){
	vec3 color = vec3(texture(sceneTexture, texCoord));

	if(useBloom) 
		color += vec3(texture(bloomTexture, texCoord)) * bloomIntensity;
	
	if(useHDR) {
		vec3 a = vec3(2.51);
		vec3 b = vec3(0.03);
		vec3 c = vec3(2.43);
		vec3 d = vec3(0.59);
		vec3 e = vec3(0.14);
		color = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
	}

	FragColor = vec4(color, 1);
}
