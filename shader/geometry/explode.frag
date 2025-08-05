#version 460 core

struct Material{
	sampler2D diffuse1;
};

in vec2 gTexCoord;

out vec4 FragColor;

uniform Material material;

void main(){
	//FragColor = vec4(1);
	FragColor = vec4(vec3(texture(material.diffuse1, gTexCoord)), 1);
}