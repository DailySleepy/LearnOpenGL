#version 460 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform bool isHorizontal;

//const int size = 5;
//float weight[size] = {
//	0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216
//};

//const int size = 10;
//float weight[size] = {
//	0.196482, 0.175713, 0.137993, 0.091257, 0.050598, 
//	0.021891, 0.007572, 0.002182, 0.000546, 0.000109
//};

const int size = 15;
float weight[size] = {
    0.159576, 0.141148, 0.115974, 0.088598, 0.062998,
    0.041672, 0.025653, 0.014696, 0.007837, 0.003891,
    0.001798, 0.000773, 0.000309, 0.000115, 0.000040
};

void main() {
	vec2 texelSize = 1.0 / textureSize(screenTexture, 0);

	vec3 color = vec3(texture(screenTexture, texCoord)) * weight[0];
	
	if(isHorizontal) {
		for(int i = 1; i < size; i++) {
			vec2 uv1 = texCoord + vec2(texelSize.x * i, 0);
			if(uv1.x < 1) 
				color += texture(screenTexture, uv1).rgb * weight[i];

			vec2 uv2 = texCoord - vec2(texelSize.x * i, 0);
			if(uv2.x > 0) 
				color += texture(screenTexture, uv2).rgb * weight[i];
		}
	}
	else {
		for(int i = 1; i < size; i++) {
			vec2 uv1 = texCoord + vec2(0, texelSize.y * i);
			if(uv1.y < 1) 
				color += texture(screenTexture, uv1).rgb * weight[i];

			vec2 uv2 = texCoord - vec2(0, texelSize.y * i);
			if(uv2.y > 0) 
				color += texture(screenTexture, uv2).rgb * weight[i];
		}
	}

	FragColor = vec4(color, 1);
}
