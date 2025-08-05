#shader vertex
#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 5) in vec3 aInstancePos;

layout(std140) uniform Matrices{
	mat4 projection;
	mat4 view;
};

uniform float R_max;

void main(){
	vec3 worldPos = aPos * R_max + aInstancePos;
	gl_Position = projection * view * vec4(worldPos, 1);
}



#shader fragment
#version 460 core

void main(){
}
