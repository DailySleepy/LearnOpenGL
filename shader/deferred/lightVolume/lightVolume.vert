#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 5) in vec3 aInstancePos;
layout(location = 6) in vec3 aColor;
layout(std140) uniform Matrices {
	mat4 projection;
	mat4 view;
};

out vec3 lightPos;
out vec3 lightColor;

uniform float R_max;

void main(){
	lightPos = aInstancePos;
	lightColor = aColor;

	vec3 worldPos = aPos * R_max + aInstancePos;
	gl_Position = projection * view * vec4(worldPos, 1);
}